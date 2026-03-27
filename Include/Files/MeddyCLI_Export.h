// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#ifndef MEDDYCLI_EXPORT_H
#define MEDDYCLI_EXPORT_H

// Include the appropriate "export header" file depending on which type of this library is being compiled.

#if __has_include(<meddycli_shared_export.h>)
#   include <meddycli_shared_export.h>
#elif __has_include(<meddycli_module_export.h>)
#   include <meddycli_module_export.h>
#elif __has_include(<meddycli_object_export.h>)
#   include <meddycli_object_export.h>
#elif __has_include(<meddycli_static_export.h>)
#   include <meddycli_static_export.h>
#else
    // Since no generated header files exist, it must mean we are an interface library (being used as a header-only library).
#   include <meddycli_include_export.h>
#endif

// Undef the generated macros that we've decided not to use.

#ifdef MEDDYCLI_DEPRECATED
#   undef MEDDYCLI_DEPRECATED
#endif

#ifdef MEDDYCLI_DEPRECATED_EXPORT
#   undef MEDDYCLI_DEPRECATED_EXPORT
#endif

#ifdef MEDDYCLI_DEPRECATED_NO_EXPORT
#   undef MEDDYCLI_DEPRECATED_NO_EXPORT
#endif

#ifdef MEDDYCLI_NO_DEPRECATED
#   undef MEDDYCLI_NO_DEPRECATED
#endif

#endif // #ifndef MEDDYCLI_EXPORT_H
