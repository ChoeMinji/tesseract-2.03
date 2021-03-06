/******************************************************************************
 **	Filename:    intproto.h
 **	Purpose:     Definition of data structures for integer protos.
 **	Author:      Dan Johnson
 **	History:     Thu Feb  7 12:58:45 1991, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#ifndef INTPROTO_H
#define INTPROTO_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "matchdefs.h"
#include "protos.h"
#include "callcpp.h"
#include "unicharset.h"

/* define order of params in pruners */
#define PRUNER_X      0
#define PRUNER_Y      1
#define PRUNER_ANGLE    2

/* definition of coordinate system offsets for each table parameter */
#define ANGLE_SHIFT (0.0)
#define X_SHIFT   (0.5)
#define Y_SHIFT   (0.5)

#define MAX_PROTO_INDEX   24
#define BITS_PER_WERD   (8 * sizeof (uinT32))
#define MAX_NUM_CONFIGS   32
#define MAX_NUM_PROTOS    512
#define PROTOS_PER_PROTO_SET  64
#define MAX_NUM_PROTO_SETS  (MAX_NUM_PROTOS / PROTOS_PER_PROTO_SET)
#define NUM_PP_PARAMS   3
#define NUM_PP_BUCKETS    64
#define NUM_CP_BUCKETS    24
#define CLASSES_PER_CP    32
#define NUM_BITS_PER_CLASS  2
#define CLASSES_PER_CP_WERD (CLASSES_PER_CP / NUM_BITS_PER_CLASS)
#define PROTOS_PER_PP_WERD  BITS_PER_WERD
#define BITS_PER_CP_VECTOR  (CLASSES_PER_CP * NUM_BITS_PER_CLASS)
#define MAX_NUM_CLASS_PRUNERS	((MAX_NUM_CLASSES + CLASSES_PER_CP - 1) /   \
				CLASSES_PER_CP)
#define WERDS_PER_CP_VECTOR (BITS_PER_CP_VECTOR / BITS_PER_WERD)
#define WERDS_PER_PP_VECTOR	((PROTOS_PER_PROTO_SET+BITS_PER_WERD-1)/    \
				BITS_PER_WERD)
#define WERDS_PER_PP		(NUM_PP_PARAMS * NUM_PP_BUCKETS *		\
				WERDS_PER_PP_VECTOR)
#define WERDS_PER_CP		(NUM_CP_BUCKETS * NUM_CP_BUCKETS *		\
				NUM_CP_BUCKETS * WERDS_PER_CP_VECTOR)
#define WERDS_PER_CONFIG_VEC	((MAX_NUM_CONFIGS + BITS_PER_WERD - 1) /    \
				BITS_PER_WERD)

typedef uinT32 CLASS_PRUNER_STRUCT
[NUM_CP_BUCKETS][NUM_CP_BUCKETS][NUM_CP_BUCKETS][WERDS_PER_CP_VECTOR];

typedef
uinT32 (*CLASS_PRUNER)[NUM_CP_BUCKETS][NUM_CP_BUCKETS][WERDS_PER_CP_VECTOR];

typedef struct
{
  inT8 A;
  uinT8 B;
  inT8 C;
  uinT8 Angle;
  uinT32 Configs[WERDS_PER_CONFIG_VEC];
}


INT_PROTO_STRUCT, *INT_PROTO;

typedef struct
{
  uinT32 ProtoPruner[NUM_PP_PARAMS][NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR];
  INT_PROTO_STRUCT Protos[PROTOS_PER_PROTO_SET];
}


PROTO_SET_STRUCT, *PROTO_SET;

typedef uinT32 CONFIG_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][4];

typedef struct
{
  uinT16 NumProtos;
  uinT8 NumProtoSets;
  uinT8 NumConfigs;
  PROTO_SET ProtoSets[MAX_NUM_PROTO_SETS];
  uinT8 *ProtoLengths;
  uinT16 ConfigLengths[MAX_NUM_CONFIGS];
}


INT_CLASS_STRUCT, *INT_CLASS;

typedef struct
{
  int NumClasses;
  int NumClassPruners;
  CLASS_TO_INDEX IndexFor;
  INDEX_TO_CLASS ClassIdFor;
  INT_CLASS Class[MAX_NUM_CLASSES];
  CLASS_PRUNER ClassPruner[MAX_NUM_CLASS_PRUNERS];
}


INT_TEMPLATES_STRUCT, *INT_TEMPLATES;

/* definitions of integer features*/
#define MAX_NUM_INT_FEATURES 512
#define INT_CHAR_NORM_RANGE  256

typedef struct
{
  uinT8 X;
  uinT8 Y;
  uinT8 Theta;
  inT8 CP_misses;
}


INT_FEATURE_STRUCT;
typedef INT_FEATURE_STRUCT *INT_FEATURE;

typedef INT_FEATURE_STRUCT INT_FEATURE_ARRAY[MAX_NUM_INT_FEATURES];

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/
/* PROTO_SET access macros*/
#define ProtoPrunerFor(S) (S->ProtoPruner)

/* INT_CLASS access macros*/
#define NumIntProtosIn(C) ((C)->NumProtos)
#define NumProtoSetsIn(C) ((C)->NumProtoSets)
#define MaxNumIntProtosIn(C)  (NumProtoSetsIn (C) * PROTOS_PER_PROTO_SET)
#define NumIntConfigsIn(C)  ((C)->NumConfigs)
#define ProtoSetIn(C,I)   ((C)->ProtoSets[I])
#define SetForProto(P)    (P / PROTOS_PER_PROTO_SET)
#define IndexForProto(P)  (P % PROTOS_PER_PROTO_SET)
//#define IllegalProto(C,P)     (P >= MaxNumIntProtosIn (C))
#define ProtoForProtoId(C,P)	(&((ProtoSetIn (C, SetForProto (P)))->	\
					Protos [IndexForProto (P)]))
#define LengthForProtoId(C,P) ((C)->ProtoLengths[P])
#define LengthForConfigId(C,c)  ((C)->ConfigLengths[c])
#define PPrunerWordIndexFor(I)	(((I) % PROTOS_PER_PROTO_SET) /		\
				PROTOS_PER_PP_WERD)
#define PPrunerBitIndexFor(I) ((I) % PROTOS_PER_PP_WERD)
#define PPrunerMaskFor(I) (1 << PPrunerBitIndexFor (I))

/* INT_TEMPLATE access macros*/
#define NumClassesIn(T)   ((T)->NumClasses)
#define NumClassPrunersIn(T)  ((T)->NumClassPruners)
#define MaxNumClassesIn(T)    (NumClassPrunersIn (T) * CLASSES_PER_CP)
#define ClassIdForIndex(T,I)  ((T)->ClassIdFor[I])
#define IndexForClassId(T,C)  ((T)->IndexFor[C])
#define LegalClassId(C)   ((C) > 0 && (C) <= MAX_CLASS_ID)
#define UnusedClassIdIn(T,C)  (IndexForClassId (T,C) == ILLEGAL_CLASS)
#define ClassForIndex(T,I)  ((T)->Class[I])
#define ClassForClassId(T,C)  (ClassForIndex (T, IndexForClassId (T, C)))
#define ClassPrunersFor(T)  ((T)->ClassPruner)
#define CPrunerIdFor(I)   ((I) / CLASSES_PER_CP)
#define CPrunerFor(T,I)   ((T)->ClassPruner [CPrunerIdFor (I)])
#define CPrunerWordIndexFor(I)  (((I) % CLASSES_PER_CP) / CLASSES_PER_CP_WERD)
#define CPrunerBitIndexFor(I) (((I) % CLASSES_PER_CP) % CLASSES_PER_CP_WERD)
#define CPrunerMaskFor(L,I) (((L)+1) << CPrunerBitIndexFor (I) * NUM_BITS_PER_CLASS)

/* DEBUG macros*/
#define PRINT_MATCH_SUMMARY 0x001
#define DISPLAY_FEATURE_MATCHES 0x002
#define DISPLAY_PROTO_MATCHES 0x004
#define PRINT_FEATURE_MATCHES 0x008
#define PRINT_PROTO_MATCHES 0x010
#define CLIP_MATCH_EVIDENCE 0x020

#define MatchDebuggingOn(D)   (D)
#define PrintMatchSummaryOn(D)    ((D) & PRINT_MATCH_SUMMARY)
#define DisplayFeatureMatchesOn(D)  ((D) & DISPLAY_FEATURE_MATCHES)
#define DisplayProtoMatchesOn(D)  ((D) & DISPLAY_PROTO_MATCHES)
#define PrintFeatureMatchesOn(D)  ((D) & PRINT_FEATURE_MATCHES)
#define PrintProtoMatchesOn(D)    ((D) & PRINT_PROTO_MATCHES)
#define ClipMatchEvidenceOn(D)    ((D) & CLIP_MATCH_EVIDENCE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
int AddIntClass(INT_TEMPLATES Templates, CLASS_ID ClassId, INT_CLASS Class);

int AddIntConfig(INT_CLASS Class);

int AddIntProto(INT_CLASS Class);

void AddProtoToClassPruner(PROTO Proto,
                           CLASS_ID ClassId,
                           INT_TEMPLATES Templates);

void AddProtoToProtoPruner(PROTO Proto, int ProtoId, INT_CLASS Class);

int BucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets);

int CircBucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets);

void UpdateMatchDisplay();

void ConvertConfig(BIT_VECTOR Config, int ConfigId, INT_CLASS Class);

void ConvertProto(PROTO Proto, int ProtoId, INT_CLASS Class);

INT_TEMPLATES CreateIntTemplates(CLASSES FloatProtos,
                                 const UNICHARSET& target_unicharset);

void DisplayIntFeature(INT_FEATURE Feature, FLOAT32 Evidence);

void DisplayIntProto(INT_CLASS Class, PROTO_ID ProtoId, FLOAT32 Evidence);

void InitIntProtoVars();

INT_CLASS NewIntClass(int MaxNumProtos, int MaxNumConfigs);

void free_int_class(INT_CLASS int_class);

INT_TEMPLATES NewIntTemplates();

void free_int_templates(INT_TEMPLATES templates);

INT_TEMPLATES ReadIntTemplates(FILE *File, BOOL8 swap);

void ShowMatchDisplay();

CLASS_ID GetClassToDebug(const char *Prompt);

void WriteIntTemplates(FILE *File, INT_TEMPLATES Templates,
                       const UNICHARSET& target_unicharset);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define        _ARGS(s) s
#else
# define        _ARGS(s) ()
#endif*/

/* intproto.c
int AddIntClass
    _ARGS((INT_TEMPLATES Templates,
  CLASS_ID ClassId,
  INT_CLASS Class));

int AddIntConfig
    _ARGS((INT_CLASS Class));

int AddIntProto
    _ARGS((INT_CLASS Class));

void AddProtoToClassPruner
    _ARGS((PROTO Proto,
  CLASS_ID ClassId,
  INT_TEMPLATES Templates));

void AddProtoToProtoPruner
    _ARGS((PROTO Proto,
  int ProtoId,
  INT_CLASS Class));

int BucketFor
    _ARGS((FLOAT32 Param,
  FLOAT32 Offset,
  int NumBuckets));

int CircBucketFor
    _ARGS((FLOAT32 Param,
  FLOAT32 Offset,
  int NumBuckets));

void UpdateMatchDisplay
    _ARGS((void));

void ConvertConfig
    _ARGS((BIT_VECTOR Config,
  int ConfigId,
  INT_CLASS Class));

void ConvertProto
    _ARGS((PROTO Proto,
  int ProtoId,
  INT_CLASS Class));

INT_TEMPLATES CreateIntTemplates
    _ARGS((CLASSES FloatProtos));

void DisplayIntFeature
    _ARGS((INT_FEATURE Feature,
  FLOAT32 Evidence));

void DisplayIntProto
    _ARGS((INT_CLASS Class,
  PROTO_ID ProtoId,
  FLOAT32 Evidence));

void InitIntProtoVars
    _ARGS((void));

INT_CLASS NewIntClass
    _ARGS((int MaxNumProtos,
  int MaxNumConfigs));

INT_TEMPLATES NewIntTemplates
    _ARGS((void));

INT_TEMPLATES ReadIntTemplates
    _ARGS((FILE *File));

void ShowMatchDisplay
    _ARGS((void));

void WriteIntTemplates
    _ARGS((FILE *File,
  INT_TEMPLATES Templates));

CLASS_ID GetClassToDebug
    _ARGS((char *Prompt));

C_COL GetMatchColorFor
    _ARGS((FLOAT32 Evidence));

#undef _ARGS
*/
#endif
