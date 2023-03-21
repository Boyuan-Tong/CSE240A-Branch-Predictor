//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Boyuan Tong";
const char *studentID   = "A15114585";
const char *email       = "btong@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

uint32_t gmask;
uint32_t lmask;
uint32_t pcmask;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
struct {
  uint8_t* pht;
  uint32_t ghistory; 
}gshare_predictor;

struct {
  uint8_t* global_pht;
  uint8_t* choice_pht;    // SN, WN, WT, ST (0, 1, 2, 4) 
                          // => SG, WG, WL, SL
  uint8_t* local_pht;
  uint32_t* local_bht;
  uint32_t ghistory;
}tournament_predictor;

struct {
  uint8_t* global_pht;
  uint8_t* choice_pht;    // SN, WN, WT, ST (0, 1, 2, 4) 
                          // => SG, WG, WL, SL
  uint8_t* local_pht;
  uint32_t* local_bht;
  uint32_t ghistory;
}custom_predictor;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

void gshare_init() {
  int size = 1 << ghistoryBits;
  gshare_predictor.ghistory = 0;
  gshare_predictor.pht = malloc(sizeof(uint8_t)*size);
  memset(gshare_predictor.pht, WN, sizeof(uint8_t)*size);
}

void tournament_init() {
  int global_size = 1 << ghistoryBits;
  int local_size = 1 << lhistoryBits;
  int pc_size = 1 << pcIndexBits;
  tournament_predictor.ghistory = 0;
  tournament_predictor.global_pht = malloc(sizeof(uint8_t)*global_size);
  memset(tournament_predictor.global_pht, WN, sizeof(uint8_t)*global_size);

  tournament_predictor.choice_pht = malloc(sizeof(uint8_t)*global_size);
  memset(tournament_predictor.choice_pht, WN, sizeof(uint8_t)*global_size);

  tournament_predictor.local_pht = malloc(sizeof(uint8_t)*local_size);
  memset(tournament_predictor.local_pht, WN, sizeof(uint8_t)*local_size);

  tournament_predictor.local_bht = malloc(sizeof(uint32_t)*pc_size);
  memset(tournament_predictor.local_bht, 0, sizeof(uint32_t)*pc_size);
}

void custom_init() {
  int global_size = 1 << ghistoryBits;
  int local_size = 1 << lhistoryBits;
  int pc_size = 1 << pcIndexBits;
  custom_predictor.ghistory = 0;
  custom_predictor.global_pht = malloc(sizeof(uint8_t)*global_size);
  memset(custom_predictor.global_pht, WN, sizeof(uint8_t)*global_size);

  custom_predictor.choice_pht = malloc(sizeof(uint8_t)*global_size);
  memset(custom_predictor.choice_pht, WN, sizeof(uint8_t)*global_size);

  custom_predictor.local_pht = malloc(sizeof(uint8_t)*local_size);
  memset(custom_predictor.local_pht, WN, sizeof(uint8_t)*local_size);

  custom_predictor.local_bht = malloc(sizeof(uint32_t)*pc_size);
  memset(custom_predictor.local_bht, 0, sizeof(uint32_t)*pc_size);
}

void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  gmask = (1 << ghistoryBits) - 1;
  lmask = (1 << lhistoryBits) - 1;
  pcmask = (1 << pcIndexBits) - 1;

  switch (bpType) {
    case GSHARE:
      gshare_init();
      break;
    case TOURNAMENT:
      tournament_init();
      break;
    case CUSTOM:
      custom_init();
      break;
    default:
      break;
  }

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

uint8_t gshare_predict(uint32_t pc) {
  uint32_t idx = (pc ^ gshare_predictor.ghistory) & gmask;
  return gshare_predictor.pht[idx] >= WT;
}

uint8_t tournament_predict(uint32_t pc) {
  uint32_t ghisbits = tournament_predictor.ghistory & gmask;
  uint32_t pcbits, lhisbits;
  if (tournament_predictor.choice_pht[ghisbits] <= WN) {
    // global predictor
    return tournament_predictor.global_pht[ghisbits] >= WT;
  } else {
    // local predictor
    pcbits = pc & pcmask;
    lhisbits = tournament_predictor.local_bht[pcbits] & lmask;
    return tournament_predictor.local_pht[lhisbits] >= WT;
  }
}

uint8_t custom_predict(uint32_t pc) {
  uint32_t idx = (pc ^ custom_predictor.ghistory) &gmask;
  uint32_t lhisbits, pcbits;
  if (custom_predictor.choice_pht[idx] <= WN) {
    // global predictor
    return custom_predictor.global_pht[idx] >= WT;
  } else {
    // local predictor
    pcbits = pc & pcmask;
    lhisbits = custom_predictor.local_bht[pcbits] & lmask;
    return custom_predictor.local_pht[lhisbits] >= WT;
  }
}

uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
      return custom_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t idx = (pc ^ gshare_predictor.ghistory) & gmask;
  if (outcome == TAKEN && gshare_predictor.pht[idx] < ST)
    gshare_predictor.pht[idx] ++;
  if (outcome == NOTTAKEN && gshare_predictor.pht[idx] > SN)
    gshare_predictor.pht[idx] --;
  gshare_predictor.ghistory = (gshare_predictor.ghistory << 1) | outcome;
}

void tournament_train(uint32_t pc, uint8_t outcome) {
  uint32_t ghisbits = tournament_predictor.ghistory & gmask;
  uint32_t pcbits = pc & pcmask;
  uint32_t lhisbits = tournament_predictor.local_bht[pcbits] & lmask;
  uint8_t gresult = tournament_predictor.global_pht[ghisbits] >= WT;
  uint8_t lresult = tournament_predictor.local_pht[lhisbits] >= WT;
  
  // update global pht
  if (outcome == TAKEN && tournament_predictor.global_pht[ghisbits] < ST)
    tournament_predictor.global_pht[ghisbits] ++;
  if (outcome == NOTTAKEN && tournament_predictor.global_pht[ghisbits] > SN)
    tournament_predictor.global_pht[ghisbits] --;

  // update local pht
  if (outcome == TAKEN && tournament_predictor.local_pht[lhisbits] < ST)
    tournament_predictor.local_pht[lhisbits] ++;
  if (outcome == NOTTAKEN && tournament_predictor.local_pht[lhisbits] > SN)
    tournament_predictor.local_pht[lhisbits] --;

  // update local history
  tournament_predictor.local_bht[pcbits] = (tournament_predictor.local_bht[pcbits] << 1) | outcome;

  // update choice pht
  if (gresult == outcome && lresult != outcome && tournament_predictor.choice_pht[ghisbits] > SN) {
    // SN == Strongly Global
    tournament_predictor.choice_pht[ghisbits] --;
  }
  if (gresult != outcome && lresult == outcome && tournament_predictor.choice_pht[ghisbits] < ST) {
    // ST == Strongly Local
    tournament_predictor.choice_pht[ghisbits] ++;
  }

  // update glocal history
  tournament_predictor.ghistory = (tournament_predictor.ghistory << 1) | outcome;
}

void custom_train(uint32_t pc, uint8_t outcome) {
  uint32_t idx = (pc ^ custom_predictor.ghistory) & gmask;
  uint32_t pcbits = pc & pcmask;
  uint32_t lhisbits = custom_predictor.local_bht[pcbits] & lmask;
  uint8_t gresult = custom_predictor.global_pht[idx] >= WT;
  uint8_t lresult = custom_predictor.local_pht[lhisbits] >= WT;
  
  // update global pht
  if (outcome == TAKEN && custom_predictor.global_pht[idx] < ST)
    custom_predictor.global_pht[idx] ++;
  if (outcome == NOTTAKEN && custom_predictor.global_pht[idx] > SN)
    custom_predictor.global_pht[idx] --;

  // update local pht
  if (outcome == TAKEN && custom_predictor.local_pht[lhisbits] < ST)
    custom_predictor.local_pht[lhisbits] ++;
  if (outcome == NOTTAKEN && custom_predictor.local_pht[lhisbits] > SN)
    custom_predictor.local_pht[lhisbits] --;

  // update local history
  custom_predictor.local_bht[pcbits] = (custom_predictor.local_bht[pcbits] << 1) | outcome;

  // update choice pht
  if (gresult == outcome && lresult != outcome && custom_predictor.choice_pht[idx] > SN) {
    // SN == Strongly Global
    custom_predictor.choice_pht[idx] --;
  }
  if (gresult != outcome && lresult == outcome && custom_predictor.choice_pht[idx] < ST) {
    // ST == Strongly Local
    custom_predictor.choice_pht[idx] ++;
  }

  // update glocal history
  custom_predictor.ghistory = (custom_predictor.ghistory << 1) | outcome;
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  switch (bpType) {
    case GSHARE:
      gshare_train(pc, outcome);
      break;
    case TOURNAMENT:
      tournament_train(pc, outcome);
      break;
    case CUSTOM:
      custom_train(pc, outcome);
      break;
    default:
      break;
  }
}

void freeall() {
  switch (bpType) {
    case GSHARE:
      free(gshare_predictor.pht);
      break;
    case TOURNAMENT:
      free(tournament_predictor.global_pht);
      free(tournament_predictor.choice_pht);
      free(tournament_predictor.local_bht);
      free(tournament_predictor.local_pht);
      break;
    case CUSTOM:
      free(custom_predictor.global_pht);
      free(custom_predictor.choice_pht);
      free(custom_predictor.local_bht);
      free(custom_predictor.local_pht);
      break;
    default:
      break;
  }
}
