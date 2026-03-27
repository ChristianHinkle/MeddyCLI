// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#ifndef MEDDYCLI_INCLUDE_EXPORT_H
#define MEDDYCLI_INCLUDE_EXPORT_H

// Since interface libraries cannot generate header files during their cmake configuration, we author our
// own "export header" file here, which defines the export macros as a noop.

#define MEDDYCLI_EXPORT
#define MEDDYCLI_NO_EXPORT

#endif // #ifndef MEDDYCLI_INCLUDE_EXPORT_H
