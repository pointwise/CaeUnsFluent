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

/****************************************************************************
*
* DISCLAIMER:
* TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, POINTWISE DISCLAIMS
* ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE, WITH REGARD TO THIS SCRIPT. TO THE MAXIMUM EXTENT PERMITTED
* BY APPLICABLE LAW, IN NO EVENT SHALL POINTWISE BE LIABLE TO ANY PARTY
* FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
* WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF
* BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE
* USE OF OR INABILITY TO USE THIS SCRIPT EVEN IF POINTWISE HAS BEEN
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGES AND REGARDLESS OF THE
* FAULT OR NEGLIGENCE OF POINTWISE.
*
***************************************************************************/
