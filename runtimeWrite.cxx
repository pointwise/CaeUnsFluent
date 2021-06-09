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
 * CAEP Plugin for FLUENT
 *
 ***************************************************************************/

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"
#include "pwpPlatform.h"

#include "fluentConstants.h"
#include <algorithm>
#include <map>
#include <math.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <utility>


// VC group stats and info
struct VCGroupStats {
    PWP_UINT32  groupBlkCells;
    PWP_UINT32  elemTypes;
    PWP_UINT32  tid;
    std::string type;
    std::string name;
};

using VCBlocks      = std::vector<PWP_UINT32>;
using VCGroupData   = std::pair<VCGroupStats, VCBlocks>;
using BlockVCMap    = std::map<PWP_UINT32, VCGroupData>;
using ShadowFaces   = std::vector<PWGM_FACESTREAM_DATA>;


// Runtime export state data
struct FLUENT_DATA {
    FLUENT_DATA() {
    }

    ~FLUENT_DATA() {
    }

    void reset()
    {
        *this = FLUENT_DATA();
    }

    // current zone id
    PWP_UINT32          zone{ 0 };

    // next available global face index
    PWP_UINT32          faceIndex{ 1 };

    // index of first face in current zone
    PWP_UINT32          faceStartIndex{ 1 };

    // next available global block index
    PWP_UINT32          blockIndex{ 1 };

    // the previously streamed face's type
    PWGM_ENUM_FACETYPE  prevFaceType{ PWGM_FACETYPE_BOUNDARY };

    // previous PWGM_HDOMAIN
    PWGM_HDOMAIN        prevDom = PWGM_HDOMAIN_INIT;

    // current PWGM_HDOMAIN
    PWGM_HDOMAIN        currDom = PWGM_HDOMAIN_INIT;

    // previous PWGM_HBLOCK
    PWGM_HBLOCK         prevBlk = PWGM_HBLOCK_INIT;

    // current zone's cell type
    PWP_UINT32          vcCellType{ FLUENT_CELL_MIXED };

    // Previous cell's VC id
    PWP_UINT32          prevVCId{ PWP_BADID };

    // Previous neighbor cell's VC id
    PWP_UINT32          prevNeighborVCId{ PWP_BADID };

    // true if a header is open
    PWP_BOOL            headerOpen{ PWP_FALSE };

    // maps VC id to its blocks
    BlockVCMap          blockVCMap;

    // cache of shadow faces
    ShadowFaces         shadowFaces;

    // file pos of open face zone header comment
    sysFILEPOS          indexPos1{};

    // file pos of open face zone header data
    sysFILEPOS          indexPos2{};
};


static inline PWP_UINT32
convertCellType(const PWGM_ENUM_ELEMTYPE pwType)
{
    switch (pwType) {
    case PWGM_ELEMTYPE_TRI:
        return FLUENT_CELL_TRI;
        break;
    case PWGM_ELEMTYPE_TET:
        return FLUENT_CELL_TET;
        break;
    case PWGM_ELEMTYPE_QUAD:
        return FLUENT_CELL_QUAD;
        break;
    case PWGM_ELEMTYPE_HEX:
        return FLUENT_CELL_HEX;
        break;
    case PWGM_ELEMTYPE_PYRAMID:
        return FLUENT_CELL_PYR;
        break;
    case PWGM_ELEMTYPE_WEDGE:
        return FLUENT_CELL_WEDGE;
        break;
    default:
        break;
    }
    return FLUENT_CELL_OTHER;
}


static inline PWP_UINT32
convertCellTypeToFace(const PWP_UINT32 pwType, PWP_BOOL is2D)
{
    PWP_UINT32 result = FLUENT_FACE_OTHER;
    switch (pwType) {
    case FLUENT_CELL_TRI:
    case FLUENT_CELL_TET:
        result = FLUENT_FACE_TRI;
        break;
    case FLUENT_CELL_QUAD:
    case FLUENT_CELL_HEX:
        result = FLUENT_FACE_QUAD;
        break;
    case FLUENT_CELL_WEDGE:
    case FLUENT_CELL_PYR:
    case FLUENT_CELL_MIXED:
        result = FLUENT_FACE_MIXED;
        break;
    case FLUENT_CELL_OTHER:
        result = FLUENT_FACE_OTHER;
        break;
    default:
        result = FLUENT_FACE_OTHER;
        break;
    }

    if (is2D && (FLUENT_FACE_TRI == result || FLUENT_FACE_QUAD == result)) {
        result = FLUENT_FACE_BAR;
    }

    return result;
}


static inline PWP_BOOL
isMixedBlock(PWGM_ELEMCOUNTS elemCnts)
{
    PWP_BOOL mixed = PWP_FALSE;
    PWP_BOOL elemsFound = PWP_FALSE;
    if (0 != PWGM_ECNT_Tet(elemCnts)) {
        elemsFound = PWP_TRUE;
    }
    if (0 != PWGM_ECNT_Hex(elemCnts)) {
        mixed = elemsFound;
        elemsFound = PWP_TRUE;
    }
    if (0 != PWGM_ECNT_Wedge(elemCnts)) {
        mixed = PWP_TRUE;
        elemsFound = PWP_TRUE;
    }
    if (0 != PWGM_ECNT_Pyramid(elemCnts)) {
        mixed = PWP_TRUE;
        elemsFound = PWP_TRUE;
    }
    if (0 != PWGM_ECNT_Tri(elemCnts)) {
        mixed = elemsFound;
        elemsFound = PWP_TRUE;
    }
    if (0 != PWGM_ECNT_Quad(elemCnts)) {
        mixed = elemsFound;
    }
    return mixed;
}


static inline PWP_BOOL
getElemsCodeBlkCells(const PWGM_HBLOCK hBlk, PWGM_ELEMCOUNTS elemCnts,
        PWP_UINT32 &resultCellCode)
{
    PWP_UINT32 cellCode = FLUENT_CELL_OTHER;
    PWP_BOOL result = PWP_TRUE;

    if (isMixedBlock(elemCnts)) {
        cellCode = FLUENT_CELL_MIXED;
    } 
    else {
    	PWGM_HELEMENT elem1 = PwBlkEnumElements(hBlk, 0);
        if (PWGM_HELEMENT_ISVALID(elem1)) {
            PWGM_ELEMDATA elemData;
            PwElemDataMod(elem1, &elemData);
            cellCode = convertCellType(elemData.type);
        }
        else {
            result = PWP_FALSE;
        }
    }

    resultCellCode = cellCode;
    return result;
}


static inline PWP_UINT32
getNeighborVCId(PWGM_FACESTREAM_DATA *data)
{
    PWP_UINT32 result = PWP_BADID;
    if (PWGM_FACETYPE_CONNECTION == data->type) {
        PWGM_ENUMELEMDATA eData;
        if (PwElemDataModEnum(PwModEnumElements(data->model,
                data->neighborCellIndex), &eData)) {
            PWP_UINT32 neighborId = PWGM_HELEMENT_PID(eData.hBlkElement);
            PWGM_CONDDATA vcNeighbor;
            PWGM_HBLOCK blkNeighbor = PwModEnumBlocks(data->model, neighborId);
            if (PwBlkCondition(blkNeighbor, &vcNeighbor)) {
                result = vcNeighbor.id;
            }
        }
    }
    return result;
}


static bool
faceTypesDiffer(PWGM_ENUM_FACETYPE face1, PWGM_ENUM_FACETYPE face2)
{
    // For this comparison, we treat connection and interior faces as equal
    return face1 != face2 && (face1 == PWGM_FACETYPE_BOUNDARY ||
        face2 == PWGM_FACETYPE_BOUNDARY);
}


static void
makeSafe(std::string &bctype, const char spaceReplacement)
{
    // For Fluent compatibility, change to lowercase and replace spaces with 
    // spaceReplacement.
    std::string::iterator nIter = bctype.begin();
    for (; bctype.end() != nIter; ++nIter) {
        if (isalpha(*nIter)) {
            *nIter = (char)tolower(*nIter);
        }
        else if (isspace(*nIter)) {
            *nIter = spaceReplacement;
        }
    }
}


static void
getSafeBC(const CAEP_RTITEM &rti, const PWGM_HDOMAIN &h, PWGM_CONDDATA &cond)
{
    if (!PwDomCondition(h, &cond) || (0 == cond.tid)) {
        // Type is unspecified - default to wall BC
        cond.tid = rti.pBCInfo[0].id;
        cond.type = rti.pBCInfo[0].phystype;
        if (0 == cond.name) {
            cond.name = "Unspecified";
            cond.id = PWGM_UNSPECIFIED_COND_ID;
        }
    }
}


static void
getSafeVC(const CAEP_RTITEM &rti, const PWGM_HBLOCK &h, PWGM_CONDDATA &cond)
{
    if (!PwBlkCondition(h, &cond) || (0 == cond.tid)) {
        // Type is unspecified - default to fluid VC
        cond.tid = rti.pVCInfo[0].id;
        cond.type = rti.pVCInfo[0].phystype;
        if (0 == cond.name) {
            cond.name = "Unspecified";
            cond.id = PWGM_UNSPECIFIED_COND_ID;
        }
    }
}


static void
writeSectionListHdr(CAEP_RTITEM &rti, va_list &arglist, SectionId id,
    const char *format, const char *sfx)
{
    // (id (format)(       <-- header
    //   data line 1
    //   data line 2
    //     ...
    //   data line N
    // ))                  <-- footer
    fprintf(rti.fp, "(%d (", id);
    ::vfprintf(rti.fp, format, arglist);
    fprintf(rti.fp, ")(%s", (sfx ? sfx : ""));
}


inline static void
writeSectionListHdr(CAEP_RTITEM &rti, SectionId id, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    writeSectionListHdr(rti, args, id, format, "\n");
    va_end(args);
}


inline static void
writeSectionListHdrNoCR(CAEP_RTITEM &rti, SectionId id, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    writeSectionListHdr(rti, args, id, format, 0);
    va_end(args);
}


static void
writeSectionListFtr(CAEP_RTITEM &rti)
{
    ::fprintf(rti.fp, "))\n");
}


static void
writeSectionLine(CAEP_RTITEM &rti, va_list &arglist, SectionId id,
    const char *format, const char *sfx)
{
    // (45 (2 fluid vcFluid) ())
    fprintf(rti.fp, "(%d (", id);
    ::vfprintf(rti.fp, format, arglist);
    fprintf(rti.fp, ")())%s", (sfx ? sfx : ""));
}


static void
writeSectionLine(CAEP_RTITEM &rti, SectionId id, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    writeSectionLine(rti, args, id, format, "\n");
    va_end(args);
}


static void
writeComment(CAEP_RTITEM &rti, const char *format, ...)
{
    fprintf(rti.fp, "(%d \"", FLUENT_COMMENT);
    va_list args;
    va_start(args, format);
    ::vfprintf(rti.fp, format, args);
    va_end(args);
    fprintf(rti.fp, "\")\n");
}


static void
writeCommentNoCR(CAEP_RTITEM &rti, const char *format, ...)
{
    fprintf(rti.fp, "(%d \"", FLUENT_COMMENT);
    va_list args;
    va_start(args, format);
    ::vfprintf(rti.fp, format, args);
    va_end(args);
    fprintf(rti.fp, "\")");
}


static void
writeFacesListHdr(CAEP_RTITEM &rti, PWP_UINT32 type)
{
    writeSectionListHdrNoCR(rti, FLUENT_FACES, "%x %x %x %x %x", rti.data->zone,
        rti.data->faceStartIndex, rti.data->faceIndex - 1, (unsigned int)type,
        rti.data->vcCellType);
}


static void
writeFacesListFtr(CAEP_RTITEM &rti)
{
    writeSectionListFtr(rti);
}


/*
face header: (13 (zoneId firstIndex lastIndex elemType))
   or
face list  : (13 (zoneId firstIndex lastIndex bcType elemType)(
                 [faceType] n0 n1 n2 cr cl
                        ...
                 [faceType] n0 n1 n2 cr cl
             ))
faceType only present if elemType = 0 (MixedFace)

A zoneId of zero indicates a declaration section with no body, and
elemType indicates the type of faces in that zone. The body of a
regular face section contains the grid connectivity, and each line
appears as follows:

    n0 n1 n2 cr cl

where n* are the defining node indices of the face, and c* are the
adjacent cell indices. This is an example of the triangular face format;
the actual number of nodes depends on the element type. The ordering of
the cell indices is important. The first cell, cr, is the cell on the
right side of the face and cl is the cell on the left side. Direction is
determined by the right-hand rule. It states that, if you curl the
fingers of your right hand in the order of the nodes, your thumb will
point to the right side of the face. In 2D grids, the ˆk vector pointing
outside the grid plane is used to determine the right-hand-side cell
(cr) from ˆk × rˆ.

If there is no adjacent cell, then either cr or cl is zero. All cells,
faces, and nodes have positive indices. For files containing only a
boundary mesh, both these values are zero.

If it is a two-dimensional grid, n2 is omitted. If the face zone is of
mixed type (elemType = 0), the body of the section will include the
face type and will appear as follows:

    type n0 n1 n2 cr cl

where type is the type of face, as defined in the following table:

    elemType    FaceType        nodes/face
        0       mixed
        2       linear              2
        3       triangular          3
        4       quadrilateral       4

    bcType     BcName
        2       interior
        3       wall
        4       pressure-inlet
        4       inlet-vent
        4       intake-fan
        5       pressure-outlet
        5       exhaust-fan
        5       outlet-vent
        7       symmetry
        8       periodic-shadow
        9       pressure-far-field
        10      velocity-inlet
        12      periodic
        14      fan
        14      porous-jump
        14      radiator
        20      mass-flow-inlet
        24      interface
        31      parent (hanging node)
        36      outflow
        37      axis
*/
static void
writeOneFace(const CAEP_RTITEM &rti, const PWGM_FACESTREAM_DATA &face)
{
    // if zone has mixed cell types, must prefix face with vertex count.
    if (FLUENT_CELL_MIXED == rti.data->vcCellType) {
        fprintf(rti.fp, "%x ", face.elemData.vertCnt);
    }
    // write the node indices
    for (PWP_UINT32 i = 0; i < face.elemData.vertCnt; ++i) {
        fprintf(rti.fp, "%x ", face.elemData.index[i] + 1);
    }
    // write the owner/neighbor cell indices
    switch (face.type) {
    case PWGM_FACETYPE_BOUNDARY:
        // Since PW boundary normals point to the interior of zone, the owner
        // cell is always the first index (cr). There is no neighbor, so second
        // index is zero (cl).
        fprintf(rti.fp, "%x %x\n", face.owner.cellIndex + 1, 0);
        break;
    case PWGM_FACETYPE_INTERIOR:
    case PWGM_FACETYPE_CONNECTION:
        fprintf(rti.fp, "%x %x\n", face.owner.cellIndex + 1,
            face.neighborCellIndex + 1);
        break;
    default:
        // SHOULD NEVER GET HERE
        fprintf(rti.fp, "%x %x\n", 0, 0);
        break;
    }
}


static void
writeReal(const CAEP_RTITEM &rti, const PWP_REAL &v, const char* prefix,
          const char* suffix)
{
    if (rti.fp && suffix && prefix) {
        fprintf(rti.fp, "%s%23.15e%s", prefix, v, suffix); 
    }
}


static void
writeBlankLine(CAEP_RTITEM &rti, const PWP_UINT32 lineLength)
{
    fprintf(rti.fp, "%*.*s\n", lineLength, lineLength, "");
}


// Write the global mesh header information
static bool
writeHeader(CAEP_RTITEM &rti, PWP_UINT32 nFaces, PWP_UINT32 nBFaces, 
    PWP_UINT32 &nNodes) 
{
    time_t rawtime;
    time(&rawtime);
    PWP_UINT32 nCells = 0;
    char timestr[50];
    strftime(timestr, 50, "%H:%M:%S  %a %b %d %Y", localtime(&rawtime));

    const PWP_UINT32 dim = (CAEPU_RT_DIM_2D(&rti) ? 2 : 3);
    nNodes = PwModVertexCount(rti.model);

    //rti. getAttribute("AppNameAndVersion", appVer, "Unknown");
    const char *val = 0;
    if (!PwModGetAttributeString(rti.model, "AppNameAndVersion", &val)) {
        val = "Pointwise";
    }
    fprintf(rti.fp, "(%d \"Exported from %s\")\n", FLUENT_HEADER, val);
    writeComment(rti, "%s", timestr);
    fprintf(rti.fp, "\n");

    writeComment(rti, "Dimension : %u", dim);
    fprintf(rti.fp, "(%d %u)\n", FLUENT_DIMENSION, dim);
    fprintf(rti.fp, "\n");

    writeComment(rti, "Number of Nodes : %u", nNodes);
    fprintf(rti.fp, "(%d (0 1 %x 0 %u))\n", FLUENT_NODES, nNodes, dim);
    fprintf(rti.fp, "\n");

    writeComment(rti, "Total Number of Faces : %u", nFaces);
    writeComment(rti, "       Boundary Faces : %u", nBFaces);
    writeComment(rti, "       Interior Faces : %u", nFaces - nBFaces);
    fprintf(rti.fp, "(%d (0 1 %x 0))\n", FLUENT_FACES, nFaces);
    fprintf(rti.fp, "\n");

    PWP_UINT32 nTets = 0;
    PWP_UINT32 nPyrs = 0;
    PWP_UINT32 nWedges = 0;
    PWP_UINT32 nHexes = 0;
    PWP_UINT32 nTris = 0;
    PWP_UINT32 nQuads = 0;
    PWP_UINT32 ndx = 0;
    PWGM_HBLOCK hBlk = PwModEnumBlocks(rti.model, 0);
    PWGM_ELEMCOUNTS elemCnts;
    while (PWGM_HBLOCK_ISVALID(hBlk) && !CAEPU_RT_IS_ABORTED(&rti)) {
        nCells += PwBlkElementCount(hBlk, &elemCnts);
        if (3 == dim) {
            nTets += PWGM_ECNT_Tet(elemCnts);
            nPyrs += PWGM_ECNT_Pyramid(elemCnts);
            nWedges += PWGM_ECNT_Wedge(elemCnts);
            nHexes += PWGM_ECNT_Hex(elemCnts);
        }
        else {
            nTris += PWGM_ECNT_Tri(elemCnts);
            nQuads += PWGM_ECNT_Quad(elemCnts);
        }
        hBlk = PwModEnumBlocks(rti.model, ++ndx);
    }

    if (3 == dim) {
        writeComment(rti, "Total Number of Cells : %u", nCells);
        writeComment(rti, "            Tet cells : %u", nTets);
        writeComment(rti, "        Pyramid cells : %u", nPyrs);
        writeComment(rti, "          Wedge cells : %u", nWedges);
        writeComment(rti, "            Hex cells : %u", nHexes);
    }
    else {
        writeComment(rti, "Total Number of Cells : %u", nCells);
        writeComment(rti, "            Tri cells : %u", nTris);
        writeComment(rti, "           Quad cells : %u", nQuads);
    }
    fprintf(rti.fp, "(%d (0 1 %x 0))\n", FLUENT_CELLS, nCells);
    fprintf(rti.fp, "\n");
    return true;
}


// Write the nodes section FLUENT_NODES(10)
static bool
writeVerts(CAEP_RTITEM &rti, const PWP_UINT32 nNodes)
{
    const PWP_UINT32 dim = (CAEPU_RT_DIM_2D(&rti) ? 2 : 3);
    writeComment(rti, "Zone %u  Number of Nodes : %u", ++rti.data->zone,
        nNodes);
    // (10 (1 1 NumNodesHex 1 dim)(
    fprintf(rti.fp, "(%d (1 1 %x 1 %u)(\n", FLUENT_NODES, nNodes, dim);
    if (caeuProgressBeginStep(&rti, nNodes)) {
        PWGM_VERTDATA vertData;
        for (PWP_UINT32 ii = 0; ii < nNodes; ++ii) {
            PwVertDataMod(PwModEnumVertices(rti.model, ii), &vertData);
            writeReal(rti, vertData.x, "", "");
            if (2 == dim) {
                // Write XY only for 2-D export
                writeReal(rti, vertData.y, " ", "\n");
            }
            else {
                // Write XYZ for 3-D export
                writeReal(rti, vertData.y, " ", "");
                writeReal(rti, vertData.z, " ", "\n");
            }
            if (!caeuProgressIncr(&rti)) {
                break;
            }
        }
        caeuProgressEndStep(&rti);
    }
    // Close out nodes section
    writeSectionListFtr(rti);
    return true;
}


static void
writeZoneEnd(CAEP_RTITEM &rti, std::string condType, std::string condName)
{
    // Make zone names safe for fluent write
    makeSafe(condType, '-');
    makeSafe(condName, '_');
    // (45 (3 interior interior-3)())
    writeSectionLine(rti, FLUENT_ZONE, "%d %s %s", rti.data->zone,
        condType.c_str(), condName.c_str());
}


static void
writeCloseFaceZone(CAEP_RTITEM &rti, const PWGM_ENUM_FACETYPE faceType)
{
    writeFacesListFtr(rti);

    sysFILEPOS eof;
    pwpFileGetpos(rti.fp, &eof);

    const PWP_UINT faceCnt = rti.data->faceIndex - rti.data->faceStartIndex;

    // write the zone comment line
    pwpFileSetpos(rti.fp, &rti.data->indexPos1);
    switch (faceType) {
    case PWGM_FACETYPE_BOUNDARY: {
        PWGM_CONDDATA condData;
        getSafeBC(rti, rti.data->prevDom, condData);
        std::string convertedType = condData.type;
        makeSafe(convertedType, '-');
        writeCommentNoCR(rti, "Zone %d %u faces %u..%u, BC: %0.40s %s = %u",
            rti.data->zone, faceCnt, rti.data->faceStartIndex,
            rti.data->faceIndex - 1, condData.name, convertedType.c_str(),
            condData.tid);
        break; }
    case PWGM_FACETYPE_INTERIOR:
    case PWGM_FACETYPE_CONNECTION:
        writeCommentNoCR(rti, "Zone %d %u faces %u..%u, Interior",
            rti.data->zone, faceCnt, rti.data->faceStartIndex,
            rti.data->faceIndex - 1);
        break;
    default:
        break;
    }

    // Close specific Face zone by first finishing header, and then writing
    // face zone close.
    pwpFileSetpos(rti.fp, &rti.data->indexPos2);
    switch (faceType) {
    case PWGM_FACETYPE_BOUNDARY: {
        PWGM_CONDDATA condData;
        getSafeBC(rti, rti.data->prevDom, condData);
        writeFacesListHdr(rti, condData.tid);
        pwpFileSetpos(rti.fp, &eof);
        writeZoneEnd(rti, condData.type, condData.name);
        break; }
    case PWGM_FACETYPE_CONNECTION:
    case PWGM_FACETYPE_INTERIOR: {
        writeFacesListHdr(rti, FLUENT_INTERIOR);
        pwpFileSetpos(rti.fp, &eof);
        // Grab the current VC name from the block to VC cache
        std::string zoneName = "interior-";
        const BlockVCMap &blockToVCs = rti.data->blockVCMap;
        BlockVCMap::const_iterator mIter = blockToVCs.find(rti.data->prevVCId);
        if (blockToVCs.end() != mIter) {
            zoneName.append(mIter->second.first.name);
        }
        if (PWP_BADID != rti.data->prevNeighborVCId) {
            mIter = blockToVCs.find(rti.data->prevNeighborVCId);
            if (blockToVCs.end() != mIter) {
                zoneName.append("-");
                zoneName.append(mIter->second.first.name);
            }
        }
        writeZoneEnd(rti, "interior", zoneName.c_str());
        break; }
    default:
        break;
    }
}


static void
writeOpenFaceZone(CAEP_RTITEM &rti)
{
    // Cannot determine header specific details until we have finished
    // writing faces. Write one blank line for the zone comment and one for the
    // zone header. Theses lines will be replaced by writeCloseFaceZone() once
    // all faces have been streamed.
    fprintf(rti.fp, "\n");
    pwpFileGetpos(rti.fp, &rti.data->indexPos1);
    writeBlankLine(rti, 128);
    pwpFileGetpos(rti.fp, &rti.data->indexPos2);
    writeBlankLine(rti, 50);
}


// returns true if face stream has transitioned from one BC group to the next.
static bool
isNewBCGroup(const CAEP_RTITEM &rti) {
    return (PWGM_HDOMAIN_ID(rti.data->currDom) != PWGM_HDOMAIN_ID(
        rti.data->prevDom)) && PWGM_HDOMAIN_ISVALID(rti.data->prevDom);
}


// Load rti.data->blockVCMap. Where blockVCMap maps a VC id to a VCGroupData
// object. See BlockVCMap, VCGroupData, and VCBlocks.
static void
processBlockVCMap(CAEP_RTITEM &rti)
{
    const PWP_UINT32 blockCount = PwModBlockCount(rti.model);

    for (PWP_UINT32 blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        PWGM_HBLOCK hBlk = PwModEnumBlocks(rti.model, blockIndex);
        PWGM_CONDDATA condData;
        PWGM_ELEMCOUNTS elemCnts;
        const PWP_UINT32 nBlkCells = PwBlkElementCount(hBlk, &elemCnts);
        PWP_UINT32 elemTypes = FLUENT_FACE_OTHER;
        getElemsCodeBlkCells(hBlk, elemCnts, elemTypes);
        getSafeVC(rti, hBlk, condData);
        BlockVCMap::iterator mIter = rti.data->blockVCMap.find(condData.id);
        // Need to add new vector for this vc
        if (rti.data->blockVCMap.end() == mIter) {
            // first block
            VCGroupStats stats = {
                nBlkCells,
                elemTypes,
                condData.tid,
                condData.type,
                condData.name
            };
            VCBlocks blocks;
            blocks.push_back(blockIndex);
            VCGroupData groupData(stats, blocks);
            rti.data->blockVCMap.insert(
                std::pair<PWP_UINT32, VCGroupData>(condData.id, groupData));
        }
        else {
            // Found existing
            VCGroupData *groupData = &(mIter->second);
            VCGroupStats *grpStats = &(groupData->first);
            if (grpStats->elemTypes != elemTypes) {
                grpStats->elemTypes = FLUENT_FACE_MIXED;
            }
            grpStats->groupBlkCells += nBlkCells;
            (groupData->second).push_back(blockIndex);
        }
    }
}


// Write the Cell zone section FLUENT_CELLS(12)
static const VCGroupStats*
writeVCZone(CAEP_RTITEM &rti, const PWP_UINT32 &vcId)
{
    const VCGroupStats *grpStats = NULL;
    const BlockVCMap &blockToVCs = rti.data->blockVCMap;
    // Write all block headers
    BlockVCMap::const_iterator mIter = blockToVCs.find(vcId);
    if (blockToVCs.end() != mIter && !CAEPU_RT_IS_ABORTED(&rti)) {
        const VCGroupData *groupData = &(mIter->second);
        grpStats = &(groupData->first);
        VCBlocks blocks = groupData->second;

        // Write block comment lines
        fprintf(rti.fp, "\n");
        writeComment(rti, "Zone %u %u cells %u..%u, VC: %0.40s %s = %i",
            rti.data->zone, grpStats->groupBlkCells, rti.data->blockIndex,
            rti.data->blockIndex + grpStats->groupBlkCells - 1,
            grpStats->name.c_str(), grpStats->type.c_str(), (int)vcId);

        // Write fluent Cell line
        fprintf(rti.fp, "(%d (%x %x %x 1 %x", FLUENT_CELLS, rti.data->zone, 
            rti.data->blockIndex,
            rti.data->blockIndex + grpStats->groupBlkCells - 1,
            grpStats->elemTypes);

        // If mixed, write cell type list
        if (0 == grpStats->elemTypes) {
            fprintf(rti.fp, ")(\n");
            PWP_UINT column = 0;
            VCBlocks::iterator vIter = blocks.begin();
            for(; vIter != blocks.end(); ++vIter) {
                PWP_UINT32 cellndx = 0;
                PWGM_HBLOCK hBlk = PwModEnumBlocks(rti.model, *vIter);
                PWGM_HELEMENT hElem = PwBlkEnumElements(hBlk, 0);
                PWGM_ELEMDATA eData;
                while (PWGM_HELEMENT_ISVALID(hElem) &&
                        !CAEPU_RT_IS_ABORTED(&rti)) {
                    PwElemDataMod(hElem, &eData);
                    if (9 == column) {
                        fprintf(rti.fp, "\n");
                        column = 1;
                    }
                    else {
                        ++column;
                    }
                    fprintf(rti.fp, " %x", convertCellType(eData.type));
                    hElem = PwBlkEnumElements(hBlk, ++cellndx);
                }
            }
            fprintf(rti.fp, "\n");
        }
        writeSectionListFtr(rti);
        rti.data->blockIndex += grpStats->groupBlkCells;
        writeZoneEnd(rti, grpStats->type, grpStats->name);
    }
    return grpStats;
}


// Invoked once by PwModStreamFaces() before the first face is streamed.
PWP_UINT32
beginCB(PWGM_BEGINSTREAM_DATA *data)
{
    CAEP_RTITEM &rti = *((CAEP_RTITEM*)data->userData);
    PWP_UINT32 nNodes;
    bool result = writeHeader(rti, data->totalNumFaces, data->numBoundaryFaces,
        nNodes) && writeVerts(rti, nNodes);
    if (result) {
        // Process blocks to VC Map. Blocks in the same VC are added to a
        // vector. Those vectors are stored in a map where the VCId is the key.
        processBlockVCMap(rti);
    }
    return result && caeuProgressBeginStep(&rti, data->totalNumFaces);
}


// Invoked by PwModStreamFaces() for each face in the grid.
PWP_UINT32
faceCB(PWGM_FACESTREAM_DATA *face)
{
    CAEP_RTITEM &rti = *((CAEP_RTITEM*)face->userData);

    if ((PWGM_FACETYPE_CONNECTION == face->type) &&
        PWGM_HDOMAIN_ISVALID(face->owner.domain)) {
        // cache the shadow face for dumping in endCB().
        rti.data->shadowFaces.push_back(*face);
        return !PwuProgressQuit(rti.pApiData->apiInfo.name);
    }

    PWP_UINT32 currentVCId = rti.data->prevVCId;
    PWP_UINT32 currentNeighborVCId = rti.data->prevNeighborVCId;

    rti.data->currDom = face->owner.domain;

    // For performance reasons, only check vc id when a new block is
    // encountered.
    if (PWGM_HBLOCK_ID(face->owner.block) !=
        PWGM_HBLOCK_ID(rti.data->prevBlk)) {
        rti.data->prevBlk = face->owner.block;
        PWGM_CONDDATA blkCondData;
        PwBlkCondition(face->owner.block, &blkCondData);
        currentVCId = blkCondData.id;
        currentNeighborVCId = getNeighborVCId(face);
    }

    // 1. Close Header Zone if a header is open and
    // -> A new face type is to be written (boundary, interior/connection), or
    // -> A new VC zone is to be written, or
    // -> A new BC type is to be written.
    if (rti.data->headerOpen) {
        if (faceTypesDiffer(face->type, rti.data->prevFaceType) ||
                currentVCId != rti.data->prevVCId || isNewBCGroup(rti)) {
            writeCloseFaceZone(rti, rti.data->prevFaceType);
            rti.data->headerOpen = PWP_FALSE;
        }
    }

    // 2. Write New VC Zone if no header is open and a new VC zone is to be
    // written.
    if (!rti.data->headerOpen && currentVCId != rti.data->prevVCId) {
        // VC zones are NOT written for connections.
        if (PWGM_FACETYPE_CONNECTION != face->type) {
            ++rti.data->zone;
            const VCGroupStats *grpStats = writeVCZone(rti, currentVCId);
            if (0 != grpStats) {
                // Update vcCellType to the element types in the the current
                // VC group.
                rti.data->vcCellType = convertCellTypeToFace(
                    grpStats->elemTypes, CAEPU_RT_DIM_2D(&rti));
            }
        }
        else {
            if (CAEPU_RT_DIM_3D(&rti)) {
                // In 3D, Connections may span blocks of varying cell type.
                // Safer to assume mixed face types here.
                rti.data->vcCellType = FLUENT_FACE_MIXED;
            }
        }
        // Even if zone isn't written, update volume condition ids. This is
        // important when the face is of type connection.
        rti.data->prevVCId = currentVCId;
        rti.data->prevNeighborVCId = currentNeighborVCId;
    }

    // 3. Open face Header only if a there is no header. All faces must be
    // enclosed in a zone header/closer.
    if (!rti.data->headerOpen) {
        ++rti.data->zone;
        rti.data->prevFaceType = face->type;
        writeOpenFaceZone(rti);
        rti.data->faceStartIndex = rti.data->faceIndex;
        rti.data->prevDom = rti.data->currDom;
        rti.data->headerOpen = PWP_TRUE;
    }

    // 4. Write current face
    writeOneFace(rti, *face);
    ++rti.data->faceIndex;

    return caeuProgressIncr(&rti);
}


// Invoked once by PwModStreamFaces() after last face is streamed.
PWP_UINT32
endCB(PWGM_ENDSTREAM_DATA *data) {
    CAEP_RTITEM &rti = *((CAEP_RTITEM*) data->userData);

    if (rti.data->headerOpen) {
        // Close out the last face zone
        writeCloseFaceZone(rti, rti.data->prevFaceType);
        rti.data->headerOpen = PWP_FALSE;
    }

    if (!rti.data->shadowFaces.empty()) {
        // Deal with the cached shadow faces
        ShadowFaces &faces = rti.data->shadowFaces;
        // Sort shadow faces using a strict ordering
        std::sort(faces.begin(), faces.end(),
            [](const PWGM_FACESTREAM_DATA &f1, const PWGM_FACESTREAM_DATA &f2)
            {
                if (PWGM_HDOMAIN_ID(f1.owner.domain) !=
                        PWGM_HDOMAIN_ID(f2.owner.domain)) {
                    // primary sort by domain id
                    return PWGM_HDOMAIN_ID(f1.owner.domain) <
                        PWGM_HDOMAIN_ID(f2.owner.domain);
                }

                if (f1.owner.cellIndex != f2.owner.cellIndex) {
                    // f1.owner.domain == f2.owner.domain
                    // secondary sort by cell index
                    return f1.owner.cellIndex < f2.owner.cellIndex;
                }

                // f1.owner.domain == f2.owner.domain AND
                // f1.owner.cellIndex == f2.owner.cellIndex
                // tertiary sort by cell-face index
                return f1.owner.cellFaceIndex < f2.owner.cellFaceIndex;
            });

        // Init the domain tracking values used to detect zone transitions.
        PWGM_HDOMAIN_SET_INVALID(rti.data->prevDom);
        PWGM_HDOMAIN_SET_INVALID(rti.data->currDom);
        ShadowFaces::iterator it = faces.begin();
        for (; faces.end() != it; ++it) {
            PWGM_FACESTREAM_DATA &face = *it;
            rti.data->currDom = face.owner.domain;
            if (PWGM_HDOMAIN_ID(rti.data->currDom) !=
                    PWGM_HDOMAIN_ID(rti.data->prevDom)) {
                // We have transitioned from one zone to the next
                if (PWGM_HDOMAIN_ISVALID(rti.data->prevDom)) {
                    // Close out the previous zone
                    writeCloseFaceZone(rti, PWGM_FACETYPE_BOUNDARY);
                }
                rti.data->prevDom = rti.data->currDom;
                ++rti.data->zone;
                rti.data->faceStartIndex = rti.data->faceIndex;
                writeOpenFaceZone(rti);
            }
            // Write the face to the current zone
            writeOneFace(rti, face);
            ++rti.data->faceIndex;
        }
        // Close out the final shadow face zone
        writeCloseFaceZone(rti, PWGM_FACETYPE_BOUNDARY);
    }

    return caeuProgressEndStep(&rti);
}


// Invoked once for each requested grid export.
PWP_BOOL
runtimeWrite(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
    const CAEP_WRITEINFO *pWriteInfo)
{
    PWP_BOOL ret = PWP_FALSE;
    if (pRti && pRti->fp && model && pWriteInfo) {
        // init plugin-defined instance data pointer
        FLUENT_DATA fluentData;
        pRti->data = &fluentData; // cppcheck-suppress autoVariables

        PWP_UINT32 cnt = 2; /* the # of MAJOR progress steps */
        // 1. Write vertices
        // 2. Write faces (VC zones are written during face writting)
        if (caeuProgressInit(pRti, cnt)) {
            // Configure the grid model to enumerate elements grouped by VC
            PwModAppendEnumElementOrder(model, PWGM_ELEMORDER_VC);
            // Stream the interior model faces first, followed by the BC faces
            ret = PwModStreamFaces(pRti->model, PWGM_FACEORDER_VCGROUPSBCLAST,
                beginCB, faceCB, endCB, pRti) && !CAEPU_RT_IS_ABORTED(pRti);
            pRti->data->reset();
            caeuProgressEnd(pRti, ret);
        }
    }
    return ret;
}


// Invoked once just after plugin is loaded into memory.
PWP_BOOL
runtimeCreate(CAEP_RTITEM * /*pRti*/)
{
    bool ret = true;
    // Publish which BC types are non-inflated/shadow.
    const char * const ShadowTypes = "Porous Jump|Fan|Radiator|Interior";
    ret = ret && caeuAssignInfoValue("ShadowBcTypes", ShadowTypes, true);
    return PWP_CAST_BOOL(ret);
}


// Invoked once just before plugin is unloaded from memory.
PWP_VOID
runtimeDestroy(CAEP_RTITEM * /*pRti*/)
{
}

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
