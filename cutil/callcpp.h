/**********************************************************************
 * File:        callcpp.h
 * Description: extern C interface calling C++ from C.
 * Author:		Ray Smith
 * Created:		Sun Feb 04 20:39:23 MST 1996
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef CALLCPP_H
#define CALLCPP_H

#include "host.h"
#include "varable.h"
#include "unichar.h"

class ScrollView;

#ifdef __cplusplus
//extern "C" {
#endif
typedef enum {
  Black,
  White,
  Red,
  Yellow,
  Green,
  Cyan,
  Blue,
  Magenta,
  Aquamarine,
  Dark_SLATE_BLUE,
  Light_BLUE,
  Medium_BLUE,
  Midnight_BLUE,
  Navy_BLUE,
  Sky_BLUE,
  Slate_BLUE,
  Steel_BLUE,
  Coral,
  Brown,
  Sandy_BROWN,
  Gold,
  GoldENROD,
  Dark_GREEN,
  Dark_OLIVE_GREEN,
  Forest_GREEN,
  Lime_GREEN,
  Pale_GREEN,
  Yellow_GREEN,
  Light_GREY,
  Dark_SLATE_GREY,
  Dim_GREY,
  Grey,
  Khaki,
  Maroon,
  Orange,
  Orchid,
  Pink,
  Plum,
  Indian_RED,
  Orange_RED,
  Violet_RED,
  Salmon,
  Tan,
  Turqoise,
  Dark_TURQUOISE,
  Violet,
  Wheat,
  Green_YELLOW
} C_COL;                         /*starbase colours */

#define INTEL       0x4949
#define MOTO        0x4d4d

extern INT_VAR_H (tess_cp_mapping0, 0, "Mappings for class pruner distance");
extern INT_VAR_H (tess_cp_mapping1, 1, "Mappings for class pruner distance");
extern INT_VAR_H (tess_cp_mapping2, 2, "Mappings for class pruner distance");
extern INT_VAR_H (tess_cp_mapping3, 3, "Mappings for class pruner distance");
extern INT_VAR_H (stopper_numbers_on, 0,
"Allow numbers to be acceptable choices");
extern INT_VAR_H (config_pruner_enabled, 0, "Turn on config pruner");
extern INT_VAR_H (feature_prune_percentile, 0, "Percent of features to use");
extern INT_VAR_H (newcp_ratings_on, 0, "Use new class pruner normalisation");
extern INT_VAR_H (record_matcher_output, 0, "Record detailed matcher info");
extern INT_VAR_H (il1_adaption_test, 0,
"Dont adapt to i/I at beginning of word");
extern double_VAR_H (permuter_pending_threshold, 0.0,
"Worst conf for using pending dictionary");
extern double_VAR_H (newcp_duff_rating, 0.30,
"Worst rating for calling real matcher");
extern double_VAR_H (newcp_prune_threshold, 1.2, "Ratio of best to prune");
extern double_VAR_H (tessedit_cp_ratio, 0.0, "Ratio from best to prune");

//Global matcher info from the class pruner.
extern inT32 cp_classes;
extern inT32 cp_bestindex;
extern inT32 cp_bestrating;
extern inT32 cp_bestconf;
extern char cp_chars[2];
extern inT32 cp_ratings[2];
extern inT32 cp_confs[2];
extern inT32 cp_maps[4];
//Global info to control writes of matcher info
extern inT32 blob_type;          //write control
extern char blob_answer[UNICHAR_LEN + 1];         //correct char
extern char *word_answer;        //correct word
extern inT32 matcher_pass;       //pass in chopper.c
extern inT32 bits_in_states;     //no of bits in states

#ifndef __UNIX__
void assert(             //recog one owrd
            int testing  //assert fail if false
           );
#endif
void setup_cp_maps();
void trace_stack();  //Trace current stack
void cprintf (                   //Trace printf
const char *format, ...          //special message
);
char *c_alloc_string(             //allocate string
                     inT32 count  //no of chars required
                    );
void c_free_string(              //free a string
                   char *string  //string to free
                  );
void *c_alloc_struct(                  //allocate memory
                     inT32 count,      //no of chars required
                     const char *name  //class name
                    );
void c_free_struct(                   //free a structure
                   void *deadstruct,  //structure to free
                   inT32 count,       //no of bytes
                   const char *name   //class name
                  );
void *c_alloc_mem_p(             //allocate permanent space
                    inT32 count  //block size to allocate
                   );
void *c_alloc_mem(             //get some memory
                  inT32 count  //no of bytes to get
                 );
void c_free_mem(                //free mem from alloc_mem
                void *oldchunk  //chunk to free
               );
void c_check_mem(                     //check consistency
                 const char *string,  //context message
                 inT8 level           //level of check
                );
ScrollView *c_create_window(                   /*create a window */
                      const char *name,  /*name/title of window */
                      inT16 xpos,        /*coords of window */
                      inT16 ypos,        /*coords of window */
                      inT16 xsize,       /*size of window */
                      inT16 ysize,       /*size of window */
                      double xmin,       /*scrolling limits */
                      double xmax,       /*to stop users */
                      double ymin,       /*getting lost in */
                      double ymax        /*empty space */
                     );
void c_line_color_index(  /*set color */
                        void *win,
                        C_COL index);
void c_move(  /*move pen */
            void *win,
            double x,
            double y);
void c_draw(  /*move pen */
            void *win,
            double x,
            double y);
void c_make_current(  /*move pen */
                    void *win);
void c_clear_window(  /*move pen */
                    void *win);
char window_wait(ScrollView* win);
void reverse32(void *ptr);
void reverse16(void *ptr);

#ifdef __cplusplus
//};
#endif
#endif
