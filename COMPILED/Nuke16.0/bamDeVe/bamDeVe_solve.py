"""
bamDeVe Supporting Solver & Analysis Engine (Zero NumPy / SciPy / PyTorch)

Provides full motion correspondence gathering, pure-Python scanline EXR decoding,
and Levenberg-Marquardt lens vignette estimation. Dynamically utilizes native
C++ routines in bamDeVe.dll via ctypes when available, with pure-Python stdlib fallbacks.
"""

from __future__ import annotations

import array
import ctypes
import json
import math
import os
import random
import re
import struct
import sys
import time
import zlib
from collections import OrderedDict
from typing import Callable, Dict, List, Optional, Tuple

try:
    import nuke
except Exception:
    nuke = None

AP1_TO_Y = (0.2722287168, 0.6740817658, 0.0536895174)
DEFAULT_SAMPLES = 12
CHAIN_LENGTHS = (8, 16, 24, 32, 48, 64, 96)
SCAN_LONG_SIDE = 384
SCAN_PAIRS = 3
SCAN_CHAINS = 3
SOLVE_LONG_SIDE = {0: 768, 1: 640}

_FLOW_COMPS = ("n_u", "n_v", "p_u", "p_v")
_LAYER_RE = re.compile(r"^(?P<layer>.+)\.(?P<comp>[np]_[uv])$")
_GAP_RE = re.compile(r"_f(\d+)(?:_|$)")
_FWD_BWD = "forward/backward"
_FWD_BWD_RE = re.compile(r"^(?P<pre>.*)(?P<dir>forward|backward)\.(?P<c>[uv])$")


# ============================================================================
# Section 1: Pure-Python EXR Decoding (Zero Native Library / Zero NumPy)
# ============================================================================

_MAGIC = 20000630
_LINES_PER_BLOCK = {0: 1, 1: 1, 2: 1, 3: 16}
_HALF, _FLOAT, _UINT = 1, 2, 0
_COMPRESSION_NAMES = {
    0: "none", 1: "rle", 2: "zips", 3: "zip", 4: "piz", 5: "pxr24",
    6: "b44", 7: "b44a", 8: "dwaa", 9: "dwab"
}


class _Short(Exception):
    pass


def _attrs(data: bytes):
    if len(data) < 8:
        raise _Short()
    if struct.unpack_from("<I", data, 0)[0] != _MAGIC:
        raise ValueError("not an EXR file")
    version = struct.unpack_from("<I", data, 4)[0]
    off = 8
    attrs = {}
    try:
        while True:
            e = data.index(b"\x00", off)
            name = data[off:e].decode("latin-1")
            off = e + 1
            if name == "":
                return version, attrs, off
            off = data.index(b"\x00", off) + 1
            size = struct.unpack_from("<I", data, off)[0]
            off += 4
            if off + size > len(data):
                raise _Short()
            attrs[name] = data[off:off + size]
            off += size
    except (ValueError, struct.error):
        raise _Short()


def _header(fh):
    data = fh.read(1 << 14)
    while True:
        try:
            return _attrs(data)
        except _Short:
            more = fh.read(max(1 << 16, len(data)))
            if not more:
                raise ValueError("truncated EXR header")
            data += more


def _channel_list(attrs):
    chd = attrs.get("channels", b"")
    p = 0
    chans = []
    while p < len(chd):
        e = chd.index(b"\x00", p)
        cn = chd[p:e].decode("latin-1")
        p = e + 1
        if cn == "":
            break
        pixtype = struct.unpack_from("<i", chd, p)[0]
        p += 16
        chans.append((cn, pixtype, 2 if pixtype == _HALF else 4))
    return chans


def _box(attrs, name):
    return struct.unpack_from("<iiii", attrs[name], 0)


def _problems(version, comp):
    problems = []
    if version & (1 << 9):
        problems.append("tiled")
    if version & ((1 << 11) | (1 << 12)):
        problems.append("deep/multi-part")
    if comp not in _LINES_PER_BLOCK:
        problems.append("compression %s" % _COMPRESSION_NAMES.get(comp, comp))
    return problems


def read_header(path: str) -> dict:
    with open(path, "rb") as fh:
        version, attrs, _ = _header(fh)
    dw = _box(attrs, "dataWindow")
    disp = _box(attrs, "displayWindow") if "displayWindow" in attrs else dw
    comp = attrs["compression"][0] if "compression" in attrs else -1
    problems = _problems(version, comp)
    return {
        "channels": [c[0] for c in _channel_list(attrs)],
        "width": disp[2] - disp[0] + 1,
        "height": disp[3] - disp[1] + 1,
        "data_window": [dw[0] - disp[0], dw[1] - disp[1], dw[2] - disp[0], dw[3] - disp[1]],
        "compression": _COMPRESSION_NAMES.get(comp, str(comp)),
        "readable": not problems,
        "problems": problems,
    }


def list_channels(path: str) -> List[str]:
    return read_header(path)["channels"]


def _rle_decompress(b: bytes) -> bytes:
    out = bytearray()
    i = 0
    n = len(b)
    while i < n:
        c = b[i] - 256 if b[i] > 127 else b[i]
        i += 1
        if c < 0:
            cnt = -c
            out += b[i:i + cnt]
            i += cnt
        else:
            out += bytes([b[i]]) * (c + 1)
            i += 1
    return bytes(out)


def _unpredict_reorder(dec: bytes) -> bytearray:
    n = len(dec)
    if n == 0:
        return bytearray()
    raw = bytearray(dec)
    accum = raw[0]
    for i in range(1, n):
        accum = (accum + raw[i] - 128) & 0xFF
        raw[i] = accum
    half = (n + 1) // 2
    out = bytearray(n)
    out[0::2] = raw[:half]
    out[1::2] = raw[half:]
    return out


def _resolve(wanted: List[str], names: List[str]) -> Dict[str, str]:
    out = {}
    for w in wanted:
        if w in names:
            out[w] = w
        else:
            m = [n for n in names if n == w or n.endswith("." + w) or n.endswith(w)]
            if m:
                out[w] = m[0]
    return out


def read_channels(path: str, wanted: List[str]) -> Tuple[Dict[str, array.array], int, int]:
    with open(path, "rb") as fh:
        data = fh.read()
    version, attrs, off = _attrs(data)
    comp = attrs["compression"][0]
    problems = _problems(version, comp)
    if problems:
        raise ValueError("EXR not supported (%s) - re-write it as Zip (16 scanline): %s"
                         % (", ".join(problems), path))
    chans = _channel_list(attrs)
    xmin, ymin, xmax, ymax = _box(attrs, "dataWindow")
    W, H = xmax - xmin + 1, ymax - ymin + 1
    names = [c[0] for c in chans]
    resolved = _resolve(wanted, names)
    want_actual = set(resolved.values())

    lines_per = _LINES_PER_BLOCK[comp]
    n_blocks = (H + lines_per - 1) // lines_per
    offsets = struct.unpack_from("<%dQ" % n_blocks, data, off)

    row_bytes = sum(c[2] * W for c in chans)
    data_bufs = {a: array.array('f', [0.0] * (H * W)) for a in want_actual}

    fmt_half = f"<{W}e"
    fmt_float = f"<{W}f"
    fmt_uint = f"<{W}I"

    for b in range(n_blocks):
        bo = offsets[b]
        y0 = struct.unpack_from("<i", data, bo)[0]
        ds = struct.unpack_from("<i", data, bo + 4)[0]
        raw = data[bo + 8:bo + 8 + ds]
        nlines = min(lines_per, ymax - y0 + 1)
        expected = row_bytes * nlines
        if ds >= expected:
            buf = raw
        elif comp == 1:
            buf = _unpredict_reorder(_rle_decompress(raw))
        else:
            buf = _unpredict_reorder(zlib.decompress(raw))

        pos = 0
        for li in range(nlines):
            row = (y0 - ymin) + li
            row_offset = row * W
            for cn, pixtype, size in chans:
                nb = W * size
                if cn in data_bufs:
                    seg = buf[pos:pos + nb]
                    if pixtype == _HALF:
                        vals = struct.unpack_from(fmt_half, seg)
                    elif pixtype == _FLOAT:
                        vals = struct.unpack_from(fmt_float, seg)
                    else:
                        vals = [float(v) for v in struct.unpack_from(fmt_uint, seg)]
                    buf_arr = data_bufs[cn]
                    buf_arr[row_offset:row_offset + W] = array.array('f', vals)
                pos += nb

    dx0, dy0, dx1, dy1 = _box(attrs, "displayWindow") if "displayWindow" in attrs else (xmin, ymin, xmax, ymax)
    DW, DH = dx1 - dx0 + 1, dy1 - dy0 + 1

    nan_val = float('nan')
    result = {}

    if (xmin, ymin, xmax, ymax) == (dx0, dy0, dx1, dy1):
        for req, act in resolved.items():
            result[req] = data_bufs[act]
    else:
        x0, x1 = max(xmin, dx0), min(xmax, dx1)
        y0, y1 = max(ymin, dy0), min(ymax, dy1)
        for req, act in resolved.items():
            disp_arr = array.array('f', [nan_val] * (DW * DH))
            if x0 <= x1 and y0 <= y1:
                src_arr = data_bufs[act]
                copy_w = x1 - x0 + 1
                for y in range(y0, y1 + 1):
                    src_idx = (y - ymin) * W + (x0 - xmin)
                    dst_idx = (y - dy0) * DW + (x0 - dx0)
                    disp_arr[dst_idx:dst_idx + copy_w] = src_arr[src_idx:src_idx + copy_w]
            result[req] = disp_arr

    return result, DW, DH


# ============================================================================
# Section 2: Mathematical Solver & Levenberg-Marquardt (Native / Pure Python)
# ============================================================================

_CPP_LIB = None


def _get_cpp_lib():
    global _CPP_LIB
    if _CPP_LIB is not None:
        return _CPP_LIB
    cur_dir = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(cur_dir, "bamDeVe.dll"),
        os.path.join(cur_dir, "build", "Release", "bamDeVe.dll"),
    ]
    if nuke is not None:
        for p in nuke.pluginPath():
            candidates.append(os.path.join(p, "bamDeVe.dll"))
    for p in candidates:
        if os.path.isfile(p):
            try:
                lib = ctypes.CDLL(p)
                lib.bamdeve_fit.argtypes = [
                    ctypes.c_int,
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.c_int,
                    ctypes.c_int,
                    ctypes.c_int,
                    ctypes.c_int,
                    ctypes.c_double,
                    ctypes.c_double,
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                    ctypes.POINTER(ctypes.c_double),
                ]
                lib.bamdeve_fit.restype = ctypes.c_int

                lib.bamdeve_downsample.argtypes = [
                    ctypes.POINTER(ctypes.c_float),
                    ctypes.c_int,
                    ctypes.c_int,
                    ctypes.c_int,
                    ctypes.POINTER(ctypes.c_float),
                ]
                lib.bamdeve_downsample.restype = None
                _CPP_LIB = lib
                return _CPP_LIB
            except Exception:
                pass
    return None


def _poly_gain(r2: float, k: List[float]) -> float:
    return 1.0 + k[0] * r2 + k[1] * r2 * r2 + (k[2] * r2 * r2 * r2 if len(k) > 2 else 0.0)


def _norm_coords(x: float, y: float, height: float, width: float) -> Tuple[float, float]:
    cx0 = (width - 1.0) * 0.5
    cy0 = (height - 1.0) * 0.5
    hd = 0.5 * math.hypot(width, height)
    inv_hd = 1.0 / hd if hd > 1e-6 else 1.0
    return (x - cx0) * inv_hd, (y - cy0) * inv_hd


def _solve_linear_system(A: List[List[float]], b: List[float]) -> Optional[List[float]]:
    n = len(b)
    M = [row[:] + [b[i]] for i, row in enumerate(A)]
    for i in range(n):
        max_r = max(range(i, n), key=lambda r: abs(M[r][i]))
        M[i], M[max_r] = M[max_r], M[i]
        pivot = M[i][i]
        if abs(pivot) < 1e-15:
            return None
        inv_p = 1.0 / pivot
        for c in range(i, n + 1):
            M[i][c] *= inv_p
        for r in range(n):
            if r != i:
                factor = M[r][i]
                if abs(factor) > 1e-15:
                    for c in range(i, n + 1):
                        M[r][c] -= factor * M[i][c]
    return [M[i][n] for i in range(n)]


def _invert_matrix(A: List[List[float]]) -> Optional[List[List[float]]]:
    n = len(A)
    M = [A[r][:] + [1.0 if r == c else 0.0 for c in range(n)] for r in range(n)]
    for i in range(n):
        max_r = max(range(i, n), key=lambda r: abs(M[r][i]))
        M[i], M[max_r] = M[max_r], M[i]
        pivot = M[i][i]
        if abs(pivot) < 1e-15:
            return None
        inv_p = 1.0 / pivot
        for c in range(i, 2 * n):
            M[i][c] *= inv_p
        for r in range(n):
            if r != i:
                factor = M[r][i]
                if abs(factor) > 1e-15:
                    for c in range(i, 2 * n):
                        M[r][c] -= factor * M[i][c]
    return [[M[r][n + c] for c in range(n)] for r in range(n)]


def _soft_l1_cost(r: List[float], f: float) -> float:
    f2 = f * f
    inv_f2 = 1.0 / f2
    total = 0.0
    for val in r:
        z = val * val * inv_f2
        total += f2 * (math.sqrt(1.0 + z) - 1.0)
    return total


def _num_jac(resid_fn: Callable[[List[float]], List[float]], theta: List[float], r0: List[float]) -> List[List[float]]:
    n = len(theta)
    m = len(r0)
    J = [[0.0] * n for _ in range(m)]
    for j in range(n):
        eps = 1e-6 + 1e-6 * abs(theta[j])
        tp = list(theta)
        tp[j] += eps
        rp = resid_fn(tp)
        inv_eps = 1.0 / eps
        for i in range(m):
            J[i][j] = (rp[i] - r0[i]) * inv_eps
    return J


def robust_lm(
    resid_fn: Callable[[List[float]], List[float]],
    n_params: int,
    f_scale: float = 0.05,
    max_iter: int = 80,
    tol: float = 1e-8,
) -> List[float]:
    theta = [0.0] * n_params
    r0 = resid_fn(theta)
    cost = _soft_l1_cost(r0, f_scale)
    lam = 1e-3

    for _ in range(max_iter):
        r = resid_fn(theta)
        J = _num_jac(resid_fn, theta, r)
        m = len(r)

        inv_f2 = 1.0 / (f_scale * f_scale)
        A = [[0.0] * n_params for _ in range(n_params)]
        g = [0.0] * n_params

        for i in range(m):
            ri = r[i]
            z = ri * ri * inv_f2
            w = (1.0 + z) ** -0.25
            w2 = w * w
            for p in range(n_params):
                J_p_w2 = J[i][p] * w2
                g[p] += J_p_w2 * ri
                for q in range(n_params):
                    A[p][q] += J_p_w2 * J[i][q]

        improved = False
        step = None

        for _inner in range(30):
            H = [[A[p][q] for q in range(n_params)] for p in range(n_params)]
            for p in range(n_params):
                H[p][p] += lam * (A[p][p] + 1e-12)

            step = _solve_linear_system(H, [-v for v in g])
            if step is None:
                lam *= 10.0
                continue

            new_th = [theta[j] + step[j] for j in range(n_params)]
            cnew = _soft_l1_cost(resid_fn(new_th), f_scale)
            if cnew < cost:
                theta, cost = new_th, cnew
                lam = max(lam * 0.5, 1e-12)
                improved = True
                break
            lam *= 4.0
            if lam > 1e10:
                break

        if step is None or not improved or math.sqrt(sum(s * s for s in step)) < tol:
            break

    return theta


def _make_resid(xp, yp, xq, yq, d, height, width, fit_aniso, order, reg_strength, floor):
    n_data = len(d)
    cx0 = (width - 1.0) * 0.5
    cy0 = (height - 1.0) * 0.5
    hd = 0.5 * math.hypot(width, height)
    inv_hd = 1.0 / hd if hd > 1e-6 else 1.0

    gxp = [(x - cx0) * inv_hd for x in xp]
    gyp = [(y - cy0) * inv_hd for y in yp]
    gxq = [(x - cx0) * inv_hd for x in xq]
    gyq = [(y - cy0) * inv_hd for y in yq]

    pen = math.sqrt(reg_strength * max(n_data, 1) / 33.0)
    rho_pts = [i / 32.0 for i in range(33)]
    t_pts = [r * r for r in rho_pts]

    def unpack(th):
        k = [th[0], th[1], th[2] if order >= 3 else 0.0]
        la = th[order] if fit_aniso else 0.0
        return k, la

    def resid(th: List[float]) -> List[float]:
        k, la = unpack(th)
        a = math.exp(la)
        inv_a = 1.0 / a
        k0, k1, k2 = k[0], k[1], k[2]

        out = [0.0] * (n_data + 33 + 33 + 2)

        for i in range(n_data):
            r2p = (a * gxp[i]) ** 2 + (gyp[i] * inv_a) ** 2
            r2q = (a * gxq[i]) ** 2 + (gyq[i] * inv_a) ** 2
            gp = max(1e-3, 1.0 + r2p * (k0 + r2p * (k1 + r2p * k2)))
            gq = max(1e-3, 1.0 + r2q * (k0 + r2q * (k1 + r2q * k2)))
            out[i] = (math.log(gq) - math.log(gp)) - d[i]

        for j in range(33):
            rho = rho_pts[j]
            t = t_pts[j]
            g = 1.0 + t * (k0 + t * (k1 + t * k2))
            dgdrho = 2.0 * k0 * rho + 4.0 * k1 * (rho ** 3) + 6.0 * k2 * (rho ** 5)
            out[n_data + j] = pen * max(0.0, dgdrho)
            out[n_data + 33 + j] = pen * max(0.0, floor - g)

        out[n_data + 66] = pen * 0.5 * k2
        out[n_data + 67] = pen * 0.5 * la
        return out

    return resid, unpack, n_data


def fit_vignette_anamorphic(
    xp, yp, xq, yq, d, height, width,
    fit_aniso: bool = True,
    order: int = 2,
    reg_strength: float = 0.5,
    floor: float = 0.08,
    f_scale: float = 0.05,
) -> dict:
    n_pts = len(d)
    if n_pts == 0:
        return {"k1": 0.0, "k2": 0.0, "k3": 0.0, "log_aniso": 0.0, "n_constraints": 0}

    cpp = _get_cpp_lib()
    if cpp is not None:
        try:
            c_xp = (ctypes.c_double * n_pts)(*xp)
            c_yp = (ctypes.c_double * n_pts)(*yp)
            c_xq = (ctypes.c_double * n_pts)(*xq)
            c_yq = (ctypes.c_double * n_pts)(*yq)
            c_d = (ctypes.c_double * n_pts)(*d)
            out_k1 = ctypes.c_double()
            out_k2 = ctypes.c_double()
            out_k3 = ctypes.c_double()
            out_la = ctypes.c_double()

            ret = cpp.bamdeve_fit(
                n_pts, c_xp, c_yp, c_xq, c_yq, c_d,
                int(width), int(height), int(order), 1 if fit_aniso else 0,
                float(reg_strength), float(floor),
                ctypes.byref(out_k1), ctypes.byref(out_k2),
                ctypes.byref(out_k3), ctypes.byref(out_la)
            )
            if ret == 0:
                return {
                    "k1": float(out_k1.value),
                    "k2": float(out_k2.value),
                    "k3": float(out_k3.value),
                    "log_aniso": float(out_la.value),
                    "n_constraints": int(n_pts),
                }
        except Exception:
            pass

    resid_fn, unpack, n_data = _make_resid(xp, yp, xq, yq, d, height, width,
                                           fit_aniso, order, reg_strength, floor)
    nparam = order + (1 if fit_aniso else 0)
    theta = robust_lm(resid_fn, nparam, f_scale=f_scale)
    k, la = unpack(theta)
    return {
        "k1": float(k[0]),
        "k2": float(k[1]),
        "k3": float(k[2]),
        "log_aniso": float(la),
        "n_constraints": int(n_data),
    }


def fit_with_uncertainty(
    xp, yp, xq, yq, d, height, width,
    sid=None,
    order: int = 2,
    reg_strength: float = 0.5,
    fit_aniso: bool = True,
    floor: float = 0.08,
) -> dict:
    res = fit_vignette_anamorphic(xp, yp, xq, yq, d, height, width, fit_aniso=fit_aniso,
                                  order=order, reg_strength=reg_strength, floor=floor)
    resid_fn, unpack, n = _make_resid(xp, yp, xq, yq, d, height, width,
                                      fit_aniso, order, reg_strength, floor)
    theta = _theta(res, order, fit_aniso)
    r0 = resid_fn(theta)
    J = _num_jac(resid_fn, theta, r0)[:n]

    abs_r = sorted(abs(r0[i]) for i in range(n))
    sigma = 1.4826 * (abs_r[len(abs_r) // 2] if abs_r else 0.0)

    P = len(theta)
    A = [[sum(J[i][r] * J[i][c] for i in range(n)) for c in range(P)] for r in range(P)]
    trace_A = sum(A[i][i] for i in range(P))
    for i in range(P):
        A[i][i] += (1e-12 * trace_A + 1e-30)

    inv_A = _invert_matrix(A)
    if inv_A is None:
        inv_A = [[1.0 if r == c else 0.0 for c in range(P)] for r in range(P)]

    cov = [[sigma * sigma * inv_A[r][c] for c in range(P)] for r in range(P)]
    if sid is not None and n:
        u_sid = len(set(sid))
        factor = n / float(max(1, u_sid))
        for r in range(P):
            for c in range(P):
                cov[r][c] *= factor

    def v_corner(th):
        k, la = unpack(th)
        return _v_corner(k, la, height, width)

    vc = v_corner(theta)
    g = [0.0] * P
    for j in range(P):
        tp = list(theta)
        tp[j] += 1e-6
        g[j] = (v_corner(tp) - vc) / 1e-6

    g_cov_g = sum(g[r] * sum(cov[r][c] * g[c] for c in range(P)) for r in range(P))

    cx0 = (width - 1.0) * 0.5
    cy0 = (height - 1.0) * 0.5
    hd = 0.5 * math.hypot(width, height)
    inv_hd = 1.0 / hd if hd > 1e-6 else 1.0

    radial_diffs = []
    for i in range(n):
        gxp = (xp[i] - cx0) * inv_hd
        gyp = (yp[i] - cy0) * inv_hd
        gxq = (xq[i] - cx0) * inv_hd
        gyq = (yq[i] - cy0) * inv_hd
        radial_diffs.append(abs((gxq * gxq + gyq * gyq) - (gxp * gxp + gyp * gyp)))
    radial_diffs.sort()
    radial = radial_diffs[len(radial_diffs) // 2] if radial_diffs else 0.0

    res.update({
        "sigma": sigma,
        "sigma_v": float(math.sqrt(max(g_cov_g, 0.0))) / max(vc, 1e-3),
        "v_corner": vc,
        "radial": float(radial),
        "n_points": int(n),
    })
    return res


def _theta(res: dict, order: int, fit_aniso: bool = True) -> List[float]:
    th = [res["k1"], res["k2"]]
    if order >= 3:
        th.append(res["k3"])
    if fit_aniso:
        th.append(res["log_aniso"])
    return th


def _v_corner(k: List[float], la: float, height: float, width: float) -> float:
    gxc, gyc = _norm_coords(0.0, 0.0, height, width)
    a = math.exp(la)
    return float(_poly_gain((a * gxc) ** 2 + (gyc / a) ** 2, k))


def _v_edges(k: List[float], la: float, height: float, width: float) -> List[float]:
    xs = [0.0, 0.0, (width - 1.0) * 0.5]
    ys = [0.0, (height - 1.0) * 0.5, 0.0]
    a = math.exp(la)
    out = []
    for x, y in zip(xs, ys):
        gx, gy = _norm_coords(x, y, height, width)
        out.append(_poly_gain((a * gx) ** 2 + (gy / a) ** 2, k))
    return out


def _heldout_compare(corr, height, width, simple, full, sigma_v, reg_strength=0.5,
                      min_gain=0.03, f_scale=0.05) -> Tuple[Optional[bool], dict]:
    hw = height * width
    start = [int(s) // hw for s in corr["sid"]]
    ids = sorted(set(start))
    if len(ids) < 2:
        return None, {"reason": "needs at least two start frames"}
    set_half = set(ids[0::2])

    def sub(cond_fn):
        res = {k: [] for k in corr}
        for i, st in enumerate(start):
            if cond_fn(st):
                for k in corr:
                    res[k].append(corr[k][i])
        return res

    def fit(c, cfg):
        return fit_vignette_anamorphic(c["xp"], c["yp"], c["xq"], c["yq"], c["d"], height, width,
                                       reg_strength=reg_strength, **cfg)

    cost, fits = {}, {}
    for name, cfg in (("simple", simple), ("full", full)):
        total = 0.0
        for is_train in (True, False):
            tr = sub(lambda s: (s in set_half) if is_train else (s not in set_half))
            te = sub(lambda s: (s not in set_half) if is_train else (s in set_half))
            res = fit(tr, cfg)
            resid_fn, _, n = _make_resid(te["xp"], te["yp"], te["xq"], te["yq"], te["d"], height, width,
                                         cfg["fit_aniso"], cfg["order"], reg_strength, 0.08)
            total += _soft_l1_cost(resid_fn(_theta(res, cfg["order"], cfg["fit_aniso"]))[:n], f_scale)
        cost[name] = total / max(1, len(corr["d"]))
        fits[name] = fit(corr, cfg)

    ve_simple = _v_edges([fits["simple"]["k1"], fits["simple"]["k2"], fits["simple"]["k3"]],
                         fits["simple"]["log_aniso"], height, width)
    ve_full = _v_edges([fits["full"]["k1"], fits["full"]["k2"], fits["full"]["k3"]],
                       fits["full"]["log_aniso"], height, width)

    diff = max(abs(f - s) / max(s, 1e-3) for f, s in zip(ve_full, ve_simple))
    use = cost["full"] < (1.0 - min_gain) * cost["simple"] and diff > 2.0 * sigma_v
    return bool(use), {"cost_simple": cost["simple"], "cost_full": cost["full"], "diff": diff,
                       "fit_full": fits["full"]}


def choose_order(corr, height, width, sigma_v, fit_aniso=True, reg_strength=0.5,
                 min_gain=0.03, f_scale=0.05) -> Tuple[int, dict]:
    use, info = _heldout_compare(corr, height, width, {"order": 2, "fit_aniso": fit_aniso},
                                 {"order": 3, "fit_aniso": fit_aniso}, sigma_v, reg_strength,
                                 min_gain, f_scale)
    if use is None:
        return 2, info
    return (3 if use else 2), {"cost2": info["cost_simple"], "cost3": info["cost_full"],
                               "corner_diff": info["diff"], "k3": info["fit_full"]["k3"]}


def choose_aniso(corr, height, width, sigma_v, order=2, reg_strength=0.5,
                 min_gain=0.03, f_scale=0.05) -> Tuple[Optional[bool], dict]:
    use, info = _heldout_compare(corr, height, width, {"order": order, "fit_aniso": False},
                                 {"order": order, "fit_aniso": True}, sigma_v, reg_strength,
                                 min_gain, f_scale)
    if use is None:
        return None, info
    return use, {"cost_sph": info["cost_simple"], "cost_ana": info["cost_full"],
                 "edge_diff": info["diff"], "log_aniso": info["fit_full"]["log_aniso"]}


# ============================================================================
# Section 3: Sampling, Caching & Correspondence Estimation
# ============================================================================

class Cancelled(Exception):
    pass


def _tick(progress, msg):
    if progress is not None and progress(msg):
        raise Cancelled(msg)


def _bilinear(img: array.array, w: int, h: int, x: float, y: float) -> float:
    if not (math.isfinite(x) and math.isfinite(y)):
        return float('nan')
    x0 = int(math.floor(x))
    y0 = int(math.floor(y))
    x1 = x0 + 1
    y1 = y0 + 1
    if x0 < 0 or y0 < 0 or x1 >= w or y1 >= h:
        return float('nan')
    wx = x - x0
    wy = y - y0
    r0 = y0 * w
    r1 = y1 * w
    v00 = img[r0 + x0]
    v01 = img[r0 + x1]
    v10 = img[r1 + x0]
    v11 = img[r1 + x1]
    top = v00 * (1.0 - wx) + v01 * wx
    bot = v10 * (1.0 - wx) + v11 * wx
    return top * (1.0 - wy) + bot * wy


def _block_mean(img: array.array, w: int, h: int, factor: int) -> Tuple[array.array, int, int]:
    if factor <= 1:
        return img, w, h
    nw = w // factor
    nh = h // factor

    cpp = _get_cpp_lib()
    if cpp is not None:
        try:
            out_arr = array.array('f', [0.0] * (nw * nh))
            c_src = (ctypes.c_float * len(img)).from_buffer(img)
            c_dst = (ctypes.c_float * len(out_arr)).from_buffer(out_arr)
            cpp.bamdeve_downsample(c_src, w, h, factor, c_dst)
            return out_arr, nw, nh
        except Exception:
            pass

    out_arr = array.array('f', [0.0] * (nw * nh))
    inv_area = 1.0 / (factor * factor)
    for by in range(nh):
        sy = by * factor
        dst_row = by * nw
        for bx in range(nw):
            sx = bx * factor
            s = 0.0
            for dy in range(factor):
                src_row = (sy + dy) * w + sx
                for dx in range(factor):
                    s += img[src_row + dx]
            out_arr[dst_row + bx] = s * inv_area
    return out_arr, nw, nh


class FrameCache:
    def __init__(self, plate_path_fn, flow_path_fn, layers, long_side=768,
                 layer_frames=None, max_frames=16):
        self.plate_path_fn = plate_path_fn
        self.flow_path_fn = flow_path_fn
        self.layers = dict(layers)
        self.layer_frames = {k: set(v) for k, v in (layer_frames or {}).items()}
        self.long_side = long_side
        self.max_frames = max_frames
        self.fac = None
        self.full_shape = None
        self.reads = 0
        self._lum = OrderedDict()
        self._flow = OrderedDict()

    def factor_for(self, long_side):
        if not long_side or self.full_shape is None:
            return 1
        return max(1, round(max(self.full_shape) / long_side))

    def _set_shape(self, shape):
        if self.fac is None:
            self.full_shape = shape
            self.fac = self.factor_for(self.long_side)

    def _put(self, od, key, val):
        od[key] = val
        od.move_to_end(key)
        while len(od) > self.max_frames:
            od.popitem(last=False)

    def lum(self, fr) -> Tuple[array.array, int, int]:
        if fr in self._lum:
            self._lum.move_to_end(fr)
            return self._lum[fr]

        path = self.plate_path_fn(fr)
        names = list_channels(path)
        want = [c for c in ("R", "G", "B") if c in names] or names[:1]
        ch, w, h = read_channels(path, want)
        self._set_shape((w, h))

        if len(want) >= 3 and all(c in ch for c in ("R", "G", "B")):
            r_ch = ch["R"]
            g_ch = ch["G"]
            b_ch = ch["B"]
            lum_arr = array.array('f', [
                AP1_TO_Y[0] * r_ch[i] + AP1_TO_Y[1] * g_ch[i] + AP1_TO_Y[2] * b_ch[i]
                for i in range(w * h)
            ])
        else:
            lum_arr = ch[want[0]]

        lum_down, nw, nh = _block_mean(lum_arr, w, h, self.fac)
        self.reads += 1
        res = (lum_down, nw, nh)
        self._put(self._lum, fr, res)
        return res

    def flow(self, fr, layer):
        got = self._flow.get(fr)
        if got is not None and layer in got:
            self._flow.move_to_end(fr)
            return got[layer]

        path = self.flow_path_fn(fr)
        names = list_channels(path)
        nu, nv, pu, pv = self.layers[layer]
        want = [c for c in (nu, nv, pu, pv) if c and c in names]
        ch, w, h = read_channels(path, want)
        self._set_shape((w, h))

        fac = self.fac
        inv_fac = 1.0 / fac if fac else 1.0

        def get_comp(name):
            if name in ch:
                down, nw, nh = _block_mean(ch[name], w, h, fac)
                return array.array('f', [v * inv_fac for v in down]), nw, nh
            return None

        fwd_u = get_comp(nu)
        fwd_v = get_comp(nv)
        bwd_u = get_comp(pu) if pu else None
        bwd_v = get_comp(pv) if pv else None

        res = ((fwd_u, fwd_v), (bwd_u, bwd_v))
        got = dict(got or {})
        got[layer] = res
        self.reads += 1
        self._put(self._flow, fr, got)
        return res


class _Corr:
    def __init__(self):
        self.parts = {"xp": [], "yp": [], "xq": [], "yq": [], "d": [], "sid": [], "off": []}

    def add(self, xp, yp, xq, yq, d, sid, off):
        self.parts["xp"].extend(xp)
        self.parts["yp"].extend(yp)
        self.parts["xq"].extend(xq)
        self.parts["yq"].extend(yq)
        self.parts["d"].extend(d)
        self.parts["sid"].extend(sid)
        self.parts["off"].extend([off] * len(sid))

    def arrays(self):
        return dict(self.parts)


def chain_offsets(stride: int) -> List[int]:
    return sorted({min(stride, max(4, stride // 2)), stride})


def collect_direct(src: FrameCache, layer, starts, stride, flow_v_sign=-1.0, fb_thresh=1.5,
                   points=30000, eps=1e-4, rng=None, progress=None):
    if rng is None:
        rng = random.Random(0)
    c = _Corr()
    nw = nh = None

    for i, f in enumerate(starts):
        _tick(progress, "pair %d/%d  (%d -> %d)" % (i + 1, len(starts), f, f + stride))
        lum0, nw, nh = src.lum(f)
        lum1, _, _ = src.lum(f + stride)
        (fwd_u_res, fwd_v_res), (bwd_u_res, bwd_v_res) = src.flow(f, layer)
        fwd_u, _, _ = fwd_u_res
        fwd_v, _, _ = fwd_v_res

        bwd_u = bwd_u_res[0] if bwd_u_res else None
        bwd_v = bwd_v_res[0] if bwd_v_res else None

        valid_idx = []
        xp_list, yp_list, xq_list, yq_list, d_list = [], [], [], [], []

        for y in range(nh):
            row_idx = y * nw
            for x in range(nw):
                idx = row_idx + x
                u = fwd_u[idx]
                v = flow_v_sign * fwd_v[idx]
                qx = x + u
                qy = y + v

                i0 = _bilinear(lum0, nw, nh, float(x), float(y))
                i1 = _bilinear(lum1, nw, nh, qx, qy)
                if not (math.isfinite(i0) and math.isfinite(i1) and i0 > eps and i1 > eps):
                    continue

                if bwd_u is not None and bwd_v is not None:
                    bu = _bilinear(bwd_u, nw, nh, qx, qy)
                    bv = flow_v_sign * _bilinear(bwd_v, nw, nh, qx, qy)
                    if math.isfinite(bu) and math.isfinite(bv):
                        err = math.hypot(u + bu, v + bv)
                        if err >= fb_thresh:
                            continue

                xp_list.append(float(x))
                yp_list.append(float(y))
                xq_list.append(float(qx))
                yq_list.append(float(qy))
                d_list.append(math.log(i1) - math.log(i0))
                valid_idx.append(idx)

        total_valid = len(valid_idx)
        if total_valid > points:
            chosen = sorted(rng.sample(range(total_valid), points))
            xp_list = [xp_list[j] for j in chosen]
            yp_list = [yp_list[j] for j in chosen]
            xq_list = [xq_list[j] for j in chosen]
            yq_list = [yq_list[j] for j in chosen]
            d_list = [d_list[j] for j in chosen]
            sids = [i * nw * nh + valid_idx[j] for j in chosen]
        else:
            sids = [i * nw * nh + v for v in valid_idx]

        c.add(xp_list, yp_list, xq_list, yq_list, d_list, sids, stride)

    return c.arrays(), (nh, nw)


def collect_chain(src: FrameCache, layer, starts, offsets, flow_v_sign=-1.0, fb_thresh=1.0,
                  points=20000, eps=1e-4, rng=None, progress=None):
    if rng is None:
        rng = random.Random(0)
    offsets = sorted(set(int(o) for o in offsets))
    max_off = max(offsets)
    c = _Corr()
    nw = nh = None

    for i, f in enumerate(starts):
        lum0, nw, nh = src.lum(f)
        total_pixels = nw * nh

        p0x = [float(x) for y in range(nh) for x in range(nw)]
        p0y = [float(y) for y in range(nh) for x in range(nw)]
        px = list(p0x)
        py = list(p0y)
        valid = [True] * total_pixels
        i0 = [lum0[idx] for idx in range(total_pixels)]

        for k in range(1, max_off + 1):
            _tick(progress, "chain %d/%d  frame %d" % (i + 1, len(starts), f + k))
            fr = f + k - 1
            (n_u_res, n_v_res), _ = src.flow(fr, layer)
            n_u, _, _ = n_u_res
            n_v, _, _ = n_v_res

            _, (p_u_res, p_v_res) = src.flow(fr + 1, layer)
            p_u = p_u_res[0] if p_u_res else None
            p_v = p_v_res[0] if p_v_res else None

            for idx in range(total_pixels):
                if not valid[idx]:
                    continue
                cur_x, cur_y = px[idx], py[idx]
                du = _bilinear(n_u, nw, nh, cur_x, cur_y)
                dv = flow_v_sign * _bilinear(n_v, nw, nh, cur_x, cur_y)
                nx = cur_x + du
                ny = cur_y + dv

                if not (math.isfinite(nx) and math.isfinite(ny) and 0.0 <= nx < nw - 1.0 and 0.0 <= ny < nh - 1.0):
                    valid[idx] = False
                    continue

                if p_u is not None and p_v is not None:
                    pu = _bilinear(p_u, nw, nh, nx, ny)
                    pv = flow_v_sign * _bilinear(p_v, nw, nh, nx, ny)
                    if math.isfinite(pu) and math.isfinite(pv):
                        step_err = math.hypot(du + pu, dv + pv)
                        if step_err >= fb_thresh:
                            valid[idx] = False
                            continue

                px[idx] = nx
                py[idx] = ny

            if k in offsets:
                lum_k, _, _ = src.lum(f + k)
                xp_list, yp_list, xq_list, yq_list, d_list, sids = [], [], [], [], [], []
                for idx in range(total_pixels):
                    if not valid[idx]:
                        continue
                    v0 = i0[idx]
                    if v0 <= eps:
                        continue
                    v1 = _bilinear(lum_k, nw, nh, px[idx], py[idx])
                    if not (math.isfinite(v1) and v1 > eps):
                        continue
                    xp_list.append(p0x[idx])
                    yp_list.append(p0y[idx])
                    xq_list.append(px[idx])
                    yq_list.append(py[idx])
                    d_list.append(math.log(v1) - math.log(v0))
                    sids.append(i * total_pixels + idx)

                if len(sids) > points:
                    chosen = sorted(rng.sample(range(len(sids)), points))
                    xp_list = [xp_list[j] for j in chosen]
                    yp_list = [yp_list[j] for j in chosen]
                    xq_list = [xq_list[j] for j in chosen]
                    yq_list = [yq_list[j] for j in chosen]
                    d_list = [d_list[j] for j in chosen]
                    sids = [sids[j] for j in chosen]

                c.add(xp_list, yp_list, xq_list, yq_list, d_list, sids, k)

    return c.arrays(), (nh, nw)


def _fit_corr(corr, h, w, order, reg_strength, fit_aniso=True):
    if len(corr["d"]) == 0:
        raise RuntimeError("No usable correspondences found across sampled frames.")
    return fit_vignette_anamorphic(corr["xp"], corr["yp"], corr["xq"], corr["yq"], corr["d"],
                                   h, w, fit_aniso=fit_aniso, order=order, reg_strength=reg_strength)


def solve_from_reads(
    plate_path_fn: Callable[[int], str],
    flow_path_fn: Callable[[int], str],
    frames: List[int],
    stride: int,
    fb_thresh: float = 1.5,
    flow_v_sign: float = -1.0,
    points_per_frame: int = 30000,
    eps: float = 1e-4,
    seed: int = 0,
    order: int = 2,
    reg_strength: float = 0.5,
    long_side: int = 768,
    u_channel: str = "n_u",
    v_channel: str = "n_v",
    bwd_u_channel: str = "p_u",
    bwd_v_channel: str = "p_v",
    progress: Optional[Callable[[str], bool]] = None,
    fit_aniso: bool = True,
) -> dict:
    src = FrameCache(plate_path_fn, flow_path_fn,
                     {"flow": (u_channel, v_channel, bwd_u_channel, bwd_v_channel)},
                     long_side=long_side, max_frames=4)
    corr, (h, w) = collect_direct(src, "flow", frames, stride, flow_v_sign, fb_thresh,
                                  points_per_frame, eps, random.Random(seed), progress)
    res = _fit_corr(corr, h, w, order, reg_strength, fit_aniso)
    res["resolution"] = [h, w]
    return res


def solve_from_stride1(
    plate_path_fn: Callable[[int], str],
    flow_path_fn: Callable[[int], str],
    frames: List[int],
    stride: int = 48,
    offsets: Optional[List[int]] = None,
    long_side: int = 640,
    flow_v_sign: float = -1.0,
    fb_thresh: float = 1.0,
    points_per_offset: int = 20000,
    eps: float = 1e-4,
    seed: int = 0,
    order: int = 2,
    reg_strength: float = 0.5,
    n_u_channel: str = "n_u",
    n_v_channel: str = "n_v",
    p_u_channel: str = "p_u",
    p_v_channel: str = "p_v",
    progress: Optional[Callable[[str], bool]] = None,
    fit_aniso: bool = True,
) -> dict:
    if offsets is None:
        offsets = chain_offsets(stride)
    src = FrameCache(plate_path_fn, flow_path_fn,
                     {"flow": (n_u_channel, n_v_channel, p_u_channel, p_v_channel)},
                     long_side=long_side, max_frames=max(offsets) + 2)
    corr, (h, w) = collect_chain(src, "flow", sorted(frames), offsets, flow_v_sign, fb_thresh,
                                 points_per_offset, eps, random.Random(seed), progress)
    res = _fit_corr(corr, h, w, order, reg_strength, fit_aniso)
    res["resolution"] = [h, w]
    return res


def detect_flow_v_sign(src: FrameCache, layer, starts, gap=1, steps=1, eps=1e-4, progress=None):
    errs = {1.0: [], -1.0: []}
    vdisp = []
    for f in starts:
        lum0, nw, nh = src.lum(f)
        for s in (1.0, -1.0):
            _tick(progress, "flow v sign  frame %d" % f)
            for y in range(nh):
                for x in range(nw):
                    px, py = float(x), float(y)
                    for k in range(steps):
                        (fwd_u, fwd_v), _ = src.flow(f + k * gap, layer)
                        du = _bilinear(fwd_u[0], nw, nh, px, py)
                        dv = s * _bilinear(fwd_v[0], nw, nh, px, py)
                        px += du
                        py += dv
                    i0 = lum0[y * nw + x]
                    lum_step, _, _ = src.lum(f + steps * gap)
                    i1 = _bilinear(lum_step, nw, nh, px, py)
                    if math.isfinite(i1) and i1 > eps and i0 > eps:
                        errs[s].append(abs(math.log(i1) - math.log(i0)))
                        if s == 1.0:
                            vdisp.append(abs(py - y))

    def med(arr):
        if not arr:
            return float('inf')
        arr.sort()
        return arr[len(arr) // 2]

    e1 = med(errs[1.0])
    e_neg1 = med(errs[-1.0])
    vd = med(vdisp) if vdisp else 0.0

    lo = 1.0 if e1 < e_neg1 else -1.0
    decided = vd >= 0.5 and (e1 < 0.9 * e_neg1 or e_neg1 < 0.9 * e1)
    return (lo if decided else None), {"err_pos": e1, "err_neg": e_neg1, "v_disp": vd}


def _score(corr, h, w, order, reg_strength):
    n = len(corr["d"])
    if n < 1000:
        return {"ok": False, "n_points": n, "sigma_v": float("inf")}
    res = fit_with_uncertainty(corr["xp"], corr["yp"], corr["xq"], corr["yq"], corr["d"], h, w,
                               sid=corr["sid"], order=order, reg_strength=reg_strength)
    res["ok"] = bool(math.isfinite(res["sigma_v"]))
    res["corr"], res["hw"] = corr, (h, w)
    return res


def scan_direct(src: FrameCache, layer, starts, stride, flow_v_sign=-1.0, fb_thresh=1.5,
                points=8000, order=2, reg_strength=0.5, seed=0, progress=None) -> dict:
    corr, (h, w) = collect_direct(src, layer, starts, stride, flow_v_sign, fb_thresh, points,
                                  rng=random.Random(seed), progress=progress)
    res = _score(corr, h, w, order, reg_strength)
    res["n_starts"] = len(starts)
    return res


def scan_chain(src: FrameCache, layer, starts, lengths, flow_v_sign=-1.0, fb_thresh=1.0,
               points=6000, order=2, reg_strength=0.5, seed=0, progress=None) -> dict:
    offs = {L: chain_offsets(L) for L in lengths}
    union = sorted(set(o for v in offs.values() for o in v))
    corr, (h, w) = collect_chain(src, layer, sorted(starts), union, flow_v_sign, fb_thresh, points,
                                 rng=random.Random(seed), progress=progress)
    out = {}
    for L in lengths:
        sub_c = {k: [] for k in corr}
        target_offs = set(offs[L])
        for i, off in enumerate(corr["off"]):
            if off in target_offs:
                for k in corr:
                    sub_c[k].append(corr[k][i])
        res = _score(sub_c, h, w, order, reg_strength)
        res["n_starts"] = len(starts)
        out[L] = res
    return out


# ============================================================================
# Section 4: Nuke Integration, Analysis & Solving Orchestration
# ============================================================================

def _upstream_read(group, input_index):
    n = group.input(input_index)
    for _ in range(64):
        if n is None or n.Class() == "Read":
            return n
        n = n.input(0)
    return None


def substitute_frame(pattern, frame):
    if "%" in pattern:
        try:
            return pattern % frame
        except (TypeError, ValueError):
            pass
    m = re.search(r"#+", pattern)
    if m:
        return pattern[:m.start()] + str(int(frame)).zfill(len(m.group(0))) + pattern[m.end():]
    return None


def pattern_path_fn(pattern):
    def fn(frame):
        path = substitute_frame(pattern, frame)
        return pattern if path is None else path
    return fn


def _path_fn(read_node):
    knob = read_node["file"]

    def fn(frame):
        raw = knob.getValue()
        path = substitute_frame(raw, frame)
        if path is not None:
            return path
        try:
            return knob.evaluate(frame)
        except TypeError:
            return raw
    return fn


def sample_frames(first, last, stride, n):
    hi = max(first, last - stride)
    if n >= (hi - first + 1):
        return list(range(first, hi + 1))
    if n <= 1:
        return [first]
    return [int(round(first + i * (hi - first) / float(n - 1))) for i in range(n)]


class _Progress:
    def __init__(self, title, total):
        self.task = nuke.ProgressTask(title) if nuke is not None else None
        self.total = max(1, total)
        self.done = 0

    def __call__(self, msg):
        self.done += 1
        if self.task:
            self.task.setProgress(min(99, int(100 * self.done / self.total)))
            self.task.setMessage(msg)
            return self.task.isCancelled()
        return False

    def close(self):
        if self.task:
            del self.task


def _flow_comp(ch):
    m = _LAYER_RE.match(ch)
    if m:
        return m.group("layer"), m.group("comp")
    m = _FWD_BWD_RE.match(ch)
    if m:
        return m.group("pre") + _FWD_BWD, ("n_" if m.group("dir") == "forward" else "p_") + m.group("c")
    return None


def _layer_gap(layer, frames):
    m = _GAP_RE.search(layer)
    if m:
        return int(m.group(1))
    steps = sorted(b - a for a, b in zip(frames, frames[1:]))
    return steps[len(steps) // 2] if steps else 1


def probe(plate_fn, flow_fn, plate_range, flow_range, tick=None):
    pf, pl_ = plate_range
    ff, fl = flow_range
    plate_frames, plate_missing = [], []
    for fr in range(pf, pl_ + 1):
        if tick is not None and tick("plate %d" % fr):
            return None
        (plate_frames if os.path.isfile(plate_fn(fr)) else plate_missing).append(fr)

    layer_frames = {}
    for fr in range(ff, fl + 1):
        if tick is not None and tick("flow %d" % fr):
            return None
        path = flow_fn(fr)
        if not os.path.isfile(path):
            continue
        try:
            hdr = read_header(path)
        except Exception:
            continue
        comps = {}
        for ch in hdr["channels"]:
            lc = _flow_comp(ch)
            if lc:
                comps.setdefault(lc[0], set()).add(lc[1])
        for layer, c in comps.items():
            if c.issuperset(_FLOW_COMPS):
                layer_frames.setdefault(layer, []).append(fr)

    layers = {L: {"frames": fr, "gap": _layer_gap(L, fr)} for L, fr in layer_frames.items()}
    an = {
        "plate": plate_frames,
        "flow_range": [ff, fl],
        "plate_range": [pf, pl_],
        "layers": {L: v["frames"] for L, v in layers.items()},
        "gaps": {L: v["gap"] for L, v in layers.items()},
    }
    return an


def solve_range(an):
    both = sorted(set(an["plate"]) & {f for fr in an["layers"].values() for f in fr})
    return (both[0], both[-1]) if both else None


def _valid_frames(an, mode, layer, stride, first, last):
    fs = set(an["layers"].get(layer, []))
    pl = set(an["plate"])
    if mode == 0:
        return [f for f in sorted(fs)
                if first <= f and f + stride <= last
                and f + stride in fs and f in pl and f + stride in pl]
    return [f for f in range(first, last - stride + 1)
            if all(f + k in fs and f + k in pl for k in range(stride + 1))]


def _pick_evenly(frames, n):
    if n <= 0 or n >= len(frames):
        return list(frames)
    idx = sorted({int(round(i * (len(frames) - 1) / float(n - 1))) for i in range(n)}) if n > 1 else [0]
    return [frames[i] for i in idx]


def _channels(layer):
    if layer.endswith(_FWD_BWD):
        pre = layer[:-len(_FWD_BWD)]
        return tuple(pre + c for c in ("forward.u", "forward.v", "backward.u", "backward.v"))
    return tuple("%s.%s" % (layer, c) for c in _FLOW_COMPS)


def fallback_options(an, first, last, stride):
    layers, plate = an["layers"], an["plate"]
    opts = []
    for layer in sorted(layers, key=lambda L: -an["gaps"][L]):
        gap, frames = an["gaps"][layer], layers[layer]
        sub = {"layers": {layer: frames}, "plate": plate}
        pairs = _valid_frames(sub, 0, layer, gap, frames[0], frames[-1] + gap)
        if pairs:
            opts.append({"label": "direct  %s  stride %d  (%d pairs)" % (layer, gap, len(pairs)),
                         "mode": 0, "layer": layer, "stride": gap})
        if gap == 1 and len(frames) >= 2:
            opts.append({"label": "chain  %s  (%d frames, stride = chain length)" % (layer, len(frames)),
                         "mode": 1, "layer": layer, "stride": None})
    return opts


def auto_setup(an, plate_fn, flow_fn, first, last, flow_v_sign=-1.0, fb_thresh=1.5,
               reg_strength=0.5, fit_aniso=True, prog=None):
    cands = []
    for layer in sorted(an["gaps"], key=lambda L: an["gaps"][L]):
        gap = an["gaps"][layer]
        if gap == 1:
            lengths = [L for L in CHAIN_LENGTHS if len(_valid_frames(an, 1, layer, L, first, last)) >= 2]
            if lengths:
                cands.append({"mode": 1, "layer": layer, "lengths": lengths})
                continue
        valid = _valid_frames(an, 0, layer, gap, first, last)
        if valid:
            cands.append({"mode": 0, "layer": layer, "stride": gap, "valid": valid})

    if not cands:
        return fallback_options(an, first, last, 64), {}

    max_len = max([max(c["lengths"]) for c in cands if c["mode"] == 1] or [0])
    src = FrameCache(plate_fn, flow_fn, {L: _channels(L) for L in an["gaps"]},
                     long_side=SCAN_LONG_SIDE, layer_frames=an["layers"],
                     max_frames=max(8, max_len + 3))

    opts = []
    for c in cands:
        layer, ch = c["layer"], _channels(c["layer"])
        fb = float(fb_thresh) * src.factor_for(SOLVE_LONG_SIDE[c["mode"]]) / float(src.fac or 1)
        if c["mode"] == 0:
            starts = _pick_evenly(c["valid"], SCAN_PAIRS)
            res = scan_direct(src, layer, starts, c["stride"], flow_v_sign, fb,
                              order=2, reg_strength=reg_strength, progress=prog)
            opts.append({"mode": 0, "layer": layer, "stride": c["stride"], "samples": min(DEFAULT_SAMPLES, len(c["valid"])),
                         "channels": ch, "pred": res["sigma_v"], "ok": res["ok"], "k1": res.get("k1"), "k2": res.get("k2"),
                         "label": "direct %s stride %d" % (layer, c["stride"])})
        else:
            longest = max(c["lengths"])
            valid = _valid_frames(an, 1, layer, longest, first, last)
            starts = _pick_evenly(valid, SCAN_CHAINS)
            by_len = scan_chain(src, layer, starts, c["lengths"], flow_v_sign, fb,
                                order=2, reg_strength=reg_strength, progress=prog)
            for L in c["lengths"]:
                res = by_len[L]
                opts.append({"mode": 1, "layer": layer, "stride": L, "samples": min(DEFAULT_SAMPLES, len(valid)),
                             "channels": ch, "pred": res["sigma_v"], "ok": res["ok"], "k1": res.get("k1"), "k2": res.get("k2"),
                             "label": "chain %s length %d" % (layer, L)})

    opts.sort(key=lambda o: (not o["ok"], o["pred"]))
    return opts, {"flow_v_sign": flow_v_sign, "fit_aniso": fit_aniso, "order": 2}


def _apply_option(group, opt):
    if "flow_mode" in group.knobs():
        group["flow_mode"].setValue(opt["mode"])
    chans = opt.get("channels") or _channels(opt["layer"])
    for comp, ch in zip(_FLOW_COMPS, chans):
        kn = "%s_channel" % comp
        if kn in group.knobs():
            group[kn].setValue(ch)
    if opt.get("stride") and "stride" in group.knobs():
        group["stride"].setValue(opt["stride"])
    if opt.get("samples") and "num_samples" in group.knobs():
        group["num_samples"].setValue(opt["samples"])


def knob_changed(group, knob):
    if knob.name() == "flow_source":
        try:
            raw = group["analysis_json"].value()
            an = json.loads(raw) if raw else None
            if an:
                opts = an.get("options", [])
                idx = int(knob.getValue())
                if 0 <= idx < len(opts):
                    _apply_option(group, opts[idx])
        except Exception:
            pass


def analyse(group, quiet=False):
    if nuke is None:
        return
    plate = _upstream_read(group, 0)
    flow = _upstream_read(group, 1)
    if plate is None or flow is None:
        nuke.message("bamDeVe: Connect a plate Read (input 0) and a flow Read (input 1).")
        return

    plate_fn = _path_fn(plate)
    flow_fn = _path_fn(flow)
    pf, pl = int(plate["first"].value()), int(plate["last"].value())
    ff, fl = int(flow["first"].value()), int(flow["last"].value())

    prog = _Progress("bamDeVe: Analysing", (pl - pf + 1) + (fl - ff + 1))
    try:
        an = probe(plate_fn, flow_fn, (pf, pl), (ff, fl), tick=prog)
    finally:
        prog.close()

    if not an:
        return

    rng = solve_range(an)
    if rng:
        group["solve_first"].setValue(rng[0])
        group["solve_last"].setValue(rng[1])
    first = int(group["solve_first"].value())
    last = int(group["solve_last"].value())

    prog = _Progress("bamDeVe: Scanning flow candidates", 20)
    try:
        options, picks = auto_setup(
            an, plate_fn, flow_fn, first, last,
            flow_v_sign=float(group["flow_v_sign"].value()),
            fb_thresh=float(group["fb_thresh"].value()),
            reg_strength=float(group["reg_strength"].value()),
            fit_aniso=bool(group["fit_aniso"].value()),
            prog=prog
        )
    finally:
        prog.close()

    an["options"] = options
    group["analysis_json"].setValue(json.dumps(an))
    report = "Analysed range: %d-%d\n%d candidates evaluated." % (first, last, len(options))
    group["analysis_info"].setValue(report)
    group["analysis_status"].setValue("analysed (%s)" % time.strftime("%Y-%m-%d %H:%M"))

    k = group["flow_source"]
    labels = [o["label"] for o in options] or ["(no usable flow found)"]
    k.setValues(labels)
    k.setValue(0)
    if options:
        _apply_option(group, options[0])

    if not quiet:
        nuke.message("Analysis complete.\nBest candidate: %s" % (options[0]["label"] if options else "None"))


def solve(group):
    if nuke is None:
        return
    plate = _upstream_read(group, 0)
    flow = _upstream_read(group, 1)
    if plate is None or flow is None:
        nuke.message("bamDeVe: Connect a plate Read (input 0) and a flow Read (input 1).")
        return

    channels = tuple(group["%s_channel" % c].value() for c in _FLOW_COMPS)
    mode = int(group["flow_mode"].getValue())
    stride = int(group["stride"].value())
    first = int(group["solve_first"].value())
    last = int(group["solve_last"].value())
    samples = int(group["num_samples"].value())

    frames = sample_frames(first, last, stride, samples)
    prog = _Progress("bamDeVe: Solving Vignette", len(frames))

    try:
        common = dict(
            plate_path_fn=_path_fn(plate),
            flow_path_fn=_path_fn(flow),
            frames=frames,
            stride=stride,
            flow_v_sign=float(group["flow_v_sign"].value()),
            order=int(group["order"].value()),
            reg_strength=float(group["reg_strength"].value()),
            fb_thresh=float(group["fb_thresh"].value()),
            fit_aniso=bool(group["fit_aniso"].value()),
            progress=prog,
            long_side=SOLVE_LONG_SIDE[mode]
        )
        if mode == 0:
            res = solve_from_reads(u_channel=channels[0], v_channel=channels[1],
                                   bwd_u_channel=channels[2], bwd_v_channel=channels[3], **common)
        else:
            res = solve_from_stride1(n_u_channel=channels[0], n_v_channel=channels[1],
                                     p_u_channel=channels[2], p_v_channel=channels[3], **common)
    finally:
        prog.close()

    group["k1"].setValue(res["k1"])
    group["k2"].setValue(res["k2"])
    if "k3" in group.knobs():
        group["k3"].setValue(res["k3"])
    group["log_aniso"].setValue(res["log_aniso"])
    group["result_info"].setValue("solved (%s)" % time.strftime("%Y-%m-%d %H:%M"))

    nuke.message("bamDeVe: Vignette solved successfully!\nk1 = %.4f\nk2 = %.4f\nk3 = %.4f\nlog_aniso = %.4f"
                 % (res["k1"], res["k2"], res["k3"], res["log_aniso"]))


def create_vignette_node(group):
    if nuke is None:
        return
    with nuke.root():
        div = nuke.createNode("bamDeVe")
        div.setName("Vignette_Baked", uncollide=True)
        div.setXYpos(group.xpos() + 120, group.ypos() + 40)
        div["k1"].setValue(group["k1"].value())
        div["k2"].setValue(group["k2"].value())
        if "k3" in div.knobs() and "k3" in group.knobs():
            div["k3"].setValue(group["k3"].value())
        div["log_aniso"].setValue(group["log_aniso"].value())
        div["mode"].setValue(2)  # divisor (V)
        if group.input(0):
            div.setInput(0, group.input(0))
        div.setSelected(True)
        return div
