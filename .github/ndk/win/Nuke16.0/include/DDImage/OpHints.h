// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_OPHINTS_H
#define DDIMAGE_OPHINTS_H

#include <type_traits>

namespace DD {
  namespace Image {

    //! Hints for ops so we know how to create tasks for them and how to execute them.
    //! In bottom-up mode, Nuke knows to create a cache when the engine method creates
    //! an Interest on an input. For top-down, we need to know this information when 
    //! generating the task tree.
    enum class OpHints {

      //! === Base flags (mutually exclusive) ===

      //! This op uses the default rules for task creation and evaluation, which is to 
      //! wait for all inputs to be fully evaluated before starting its own computation.
      //! Inputs will be evaluated according to the bounding box and channels requested
      //! by connected downstream ops, so it is important for the downstream ops to 
      //! request only the data they will actually fetch from their inputs to avoid 
      //! over-rendering. For ops that support it, the evaluation will be divided between
      //! multiple threads and performed in parallel (currently limited to non-planar 2D Iops).
      eDefault = 0x00,

      //! This op does not need to wait for inputs to be fully evaluated before 
      //! beginning its own evaluation. When a series of eChainable ops are
      //! connected together they will be combined into a single task "bubble" and
      //! evaluated lazily by the bottom-up renderer. The task is considered to be 
      //! complete when the final op in the chain has been fully evaluated. Only 
      //! the results of the final op in the task will be cached for downstream 
      //! ops to use. This hint will yield the best results for ops such as PixelIops
      //! that pass relatively small amounts of data (e.g. a scanline) to each other
      //! and modify their input data in-place instead of creating new outputs.
      eChainable = 0x01,

      //! This op should not be rendered in top-down order. 
      //! It will instead be lazily evaluated in bottom-up order when downstream
      //! ops try to fetch them. This hint should be used for ops that cannot
      //! accurately predict which regions and/or channels they will need to
      //! request from their inputs until they are evaluated. The top-down system 
      //! will render inputs based on the region and/or channels requested
      //! by their connected downstream ops. If this information is incorrect
      //! then over-rendering may occur. The eIgnore hint can be used to
      //! avoid this problem, but is not guaranteed to provide a performance
      //! improvement as its use must be balanced against the increased thread 
      //! synchronisation that may occur as a result of rendering the op lazily.
      eIgnore = 0x02,   

      //! === Modifier flags (can be combined with a Base flag) ===

      //! This op's inputs should not be rendered in top-down order.
      //! They will instead be lazily evaluated in bottom-up order when downstream
      //! ops try to fetch them. This hint is intended for use alongside eIgnore 
      //! and provides a way to ignore an entire sub-graph if multiple ops are
      //! over-rendering as a result of the op at the end of the graph being unable
      //! to request the correct region and/or channels in top-down mode.
      eIgnoreInputs = 0x04,

      //! This op should not be rendered with multiple scanline threads. This hint
      //! should be used in the rare cases where multi-threading an op's evaluation 
      //! would result in a loss of performance.
      eSingleThread = 0x08,

      //! This op has internal multithreading, so the top-down system should try
      //! to avoid running it in parallel with other ops. When an op completes, 
      //! any successors with this hint will be run serially rather than in parallel
      //! to get the maximum benefit from the op's internal multithreading.      
      eInternalMultithreaded = 0x10,

      //! This op is not part of the top-down system, so does not require its inputs
      //! to be cached when it is acting as a terminal op (it receives the final output
      //! from an evaluated graph). Inputs will instead be lazily evaluated in bottom-up
      //! order when the op tries to fetch them.
      eUnsupportedTerminal = 0x20,
    };

    inline constexpr OpHints operator | (OpHints lhs, OpHints rhs)
    {
        using T = std::underlying_type_t <OpHints>;
        return static_cast<OpHints>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }
        
    inline OpHints& operator |= (OpHints& lhs, OpHints rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr OpHints operator & (OpHints lhs, OpHints rhs)
    {
        using T = std::underlying_type_t <OpHints>;
        return static_cast<OpHints>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    inline OpHints& operator &= (OpHints& lhs, OpHints rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }
      
  }
}

#endif
