/****************************************************************************
 *
 * (C) 2021 Cadence Design Systems, Inc. All rights reserved worldwide.
 *
 * This sample source code is not supported by Cadence Design Systems, Inc.
 * It is provided freely for demonstration purposes only.
 * SEE THE WARRANTY DISCLAIMER AT THE BOTTOM OF THIS FILE.
 *
 ***************************************************************************/
/****************************************************************************
 *
 * FLUENT constants
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

/****************************************************************************
 *
 * This file is licensed under the Cadence Public License Version 1.0 (the
 * "License"), a copy of which is found in the included file named "LICENSE",
 * and is distributed "AS IS." TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
 * LAW, CADENCE DISCLAIMS ALL WARRANTIES AND IN NO EVENT SHALL BE LIABLE TO
 * ANY PARTY FOR ANY DAMAGES ARISING OUT OF OR RELATING TO USE OF THIS FILE.
 * Please see the License for the full text of applicable terms.
 *
 ****************************************************************************/
