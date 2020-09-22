/****************************************************************************
 *
 * FLUENT constants
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2020 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _FLUENTCONSTANTS_H_
#define _FLUENTCONSTANTS_H_

enum SectionId {
    FLUENT_CELLS = 12,
    FLUENT_FACES = 13,
    FLUENT_NODES = 10,
    FLUENT_ZONE = 45,             // region where a BC is defined
    FLUENT_COMMENT = 0,
    FLUENT_HEADER = 1,
    FLUENT_DIMENSION = 2,
    FLUENT_PERIODIC_SHADOW = 18,
    FLUENT_INTERIOR = 2,          // interior face
};

enum CellType {
    FLUENT_CELL_TRI = 1,
    FLUENT_CELL_QUAD = 3,
    FLUENT_CELL_TET = 2,
    FLUENT_CELL_WEDGE = 6,
    FLUENT_CELL_HEX = 4,
    FLUENT_CELL_PYR = 5,
    FLUENT_CELL_MIXED = 0,
    FLUENT_CELL_OTHER = 7,
};

enum FaceType {
    FLUENT_FACE_BAR = 2,
    FLUENT_FACE_TRI = 3,
    FLUENT_FACE_QUAD = 4,
    FLUENT_FACE_MIXED = 0,
    FLUENT_FACE_OTHER = 5,
};

#endif
