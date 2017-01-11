/*
 * hitFTOF - registers hits for forward Time-Of-Flight
 *
 *        This is a part of the hits package for the
 *        HDGeant simulation program for Hall D.
 *
 *        version 1.0         -Richard Jones July 16, 2001
 *
 * changes:     -B. Zihlmann June 19. 2007
 *          add hit position to north and south hit structure
 *          set THRESH_MEV to zero to NOT concatenate hits.
 *
 * changes: Wed Jun 20 13:19:56 EDT 2007 B. Zihlmann 
 *          add ipart to the function hitforwardTOF
 *
 * Programmer's Notes:
 * -------------------
 * 1) In applying the attenuation to light propagating down to both ends
 *    of the counters, there has to be some point where the attenuation
 *    factor is 1.  I chose it to be the midplane, so that in the middle
 *    of the counter both ends see the unattenuated dE values.  Closer to
 *    either end, that end has a larger dE value and the opposite end a
 *    lower dE value than the actual deposition.
 * 2) In applying the propagation delay to light propagating down to the
 *    ends of the counters, there has to be some point where the timing
 *    offset is 0.  I chose it to be the midplane, so that for hits in
 *    the middle of the counter the t values measure time-of-flight from
 *    the t=0 of the event.  For hits closer to one end, that end sees
 *    a t value smaller than its true time-of-flight, and the other end
 *    sees a value correspondingly larger.  The average is the true tof.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <HDDM/hddm_s.h>
#include <geant3.h>
#include <bintree.h>
#include <gid_map.h>

#include "calibDB.h"

extern s_HDDM_t* thisInputEvent;

// plastic scintillator specific constants
static float ATTEN_LENGTH =   150;
static float C_EFFECTIVE  =   15.0;
static float BAR_LENGTH   =   252.0; // length of the bar

// kinematic constants
static float TWO_HIT_RESOL =  25.;// separation time between two different hits

static float THRESH_MEV    =  0.;  // do not throw away any hits, one can do that later

// maximum particle tracks per counter
static int TOF_MAX_HITS    = 25;  // was 100 changed to 25

// maximum MC hits per paddle
static int TOF_MAX_PAD_HITS   = 25;

// Note by RTJ:
// This constant "MAX_HITS" is a convenience constant
// that I introduced to help with preallocating arrays
// to hold hits.  It is NOT a tunable simulation parameter.
// Do NOT MODIFY, either its name or its role in the code!
#define MAX_HITS 1000


// top level pointer of FTOF hit tree
binTree_t* forwardTOFTree = 0;

static int counterCount = 0;
static int pointCount = 0;
static int initialized = 0;


/* register hits during tracking (from gustep) */
// track is   ITRA  from GEANT
// stack is   ISTAK from GEANT
// history is ISTORY from GEANT User flag for current track history (reset to 0 in GLTRAC)

void hitForwardTOF (float xin[4], float xout[4],
                    float pin[5], float pout[5], float dEsum,
                    int track, int stack, int history, int ipart) {
  float x[3], t;
  //  float dx[3];
  //float dr;
  // float dEdx;  commented out to avoid compiler warnings 4/26/2015 DL
  float xlocal[3];
  float xftof[3];
  float zeroHat[] = {0,0,0};

  if (!initialized) {


    mystr_t strings[50];
    float values[50];
    int nvalues = 50;
    int status = GetConstants("TOF/tof_parms", &nvalues, values, strings);

    if (!status) {

      int ncounter = 0;
      int i;
      for ( i=0;i<(int)nvalues;i++){
        //printf("%d %s \n",i,strings[i].str);
        if (!strcmp(strings[i].str,"TOF_ATTEN_LENGTH")) {
          ATTEN_LENGTH  = values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_C_EFFECTIVE")) {
          C_EFFECTIVE   = values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_PADDLE_LENGTH")) {
          BAR_LENGTH    = values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_TWO_HIT_RESOL")) {
          TWO_HIT_RESOL = values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_THRESH_MEV")) {
          THRESH_MEV    =  values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_MAX_HITS")){
          TOF_MAX_HITS      = (int)values[i];
          ncounter++;
        }
        if (!strcmp(strings[i].str,"TOF_MAX_PAD_HITS")) {
          TOF_MAX_PAD_HITS  = (int)values[i];
          ncounter++;
        }
      }
      if (ncounter==7){
        printf("TOF: ALL parameters loaded from Data Base\n");
      } else if (ncounter<7){
        printf("TOF: NOT ALL necessary parameters found in Data Base %d out of 7\n",ncounter);
      } else {
        printf("TOF: SOME parameters found more than once in Data Base\n");
      }

    }
    initialized = 1;

  }



  // getplane is coded in
  // src/programs/Simulation/HDGeant/hddsGeant3.F
  // this file is automatically generated from the geometry file
  // written in xml format
  // NOTE: there are three files hddsGeant3.F with the same name in
  //       the source code tree namely
  //       1) src/programs/Utilities/geantbfield2root/hddsGeant3.F
  //       2) src/programs/Simulation/HDGeant/hddsGeant3.F
  //       3) src/programs/Simulation/hdds/hddsGeant3.F
  //
  //          while 2) and 3) are identical 1) is a part of 2) and 3)
  int plane = getplane_wrapper_();
  
  // calculate mean location of track and mean time in [ns] units
  // the units of xin xout and x are in [cm]
  x[0] = (xin[0] + xout[0])/2;
  x[1] = (xin[1] + xout[1])/2;
  x[2] = (xin[2] + xout[2])/2;
  t    = (xin[3] + xout[3])/2 * 1e9;
  
  // tranform the the global x coordinate into the local coordinate of the top_volume FTOF
  // defined in the geometry file src/programs/Simulation/hdds/ForwardTOF_HDDS.xml
  // the function transform Coord is defined in src/programs/Simulation/HDGeant/hitutil/hitutil.F
  transformCoord(x,"global",xlocal,"FTOF");
  transformCoord(zeroHat,"local",xftof,"FTOF");
  
  // track vector of this step
  //dx[0] = xin[0] - xout[0];
  //dx[1] = xin[1] - xout[1];
  //dx[2] = xin[2] - xout[2];
  // length of the track of this step
  //dr = sqrt(dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2]);
  // calculate dEdx only if track length is >0.001 cm
  
  // The following commented out to avoid compiler warnings 4/26/2015 DL
//   if (dr > 1e-3) {
//     dEdx = dEsum/dr;
//   }
//   else {
//     dEdx = 0;
//   }

  /* post the hit to the truth tree */
  // in other words: store the GENERATED track information
  
  if ((history == 0) && (plane == 0)) {
    
    // save all tracks from particles that hit the first plane of FTOF
    // save the generated "true" values
    
    int mark = (1<<30) + pointCount;
    
    // getTwig is defined in src/programs/Simulation/HDGeant/bintree.c
    // the two arguments are a pointer to a pointer and an integer
    
    void** twig = getTwig(&forwardTOFTree, mark);
    if (*twig == 0) {
      // make_s_ForwardTOF is defined in src/programs/Analysis/hddm/hddm_s.h
      // and coded in src/programs/Analysis/hddm/hddm_s.c
      // the same holds for make_s_FtofTruthPoints
      
      // make_s_ForwardTOF returns pointer to structure s_ForwardTOF generated memory 
      // tof->ftofCoutners and tof-> ftofTruthPoints are initialized already
      
      s_ForwardTOF_t* tof = *twig = make_s_ForwardTOF();
      s_FtofTruthPoints_t* points = make_s_FtofTruthPoints(1);
      tof->ftofTruthPoints = points;
      int a = thisInputEvent->physicsEvents->in[0].reactions->in[0].vertices->in[0].products->mult;
      points->in[0].primary = (track <= a && stack == 0);
      points->in[0].track = track;
      points->in[0].x = x[0];
      points->in[0].y = x[1];
      points->in[0].z = x[2];
      points->in[0].t = t;
      points->in[0].px = pin[0]*pin[4];
      points->in[0].py = pin[1]*pin[4];
      points->in[0].pz = pin[2]*pin[4];
      points->in[0].E = pin[3];
      points->in[0].ptype = ipart;
      points->in[0].trackID = make_s_TrackID();
      points->in[0].trackID->itrack = gidGetId(track);
      points->mult = 1;
      pointCount++;
    }
  }
  
  /* post the hit to the hits tree, mark slab as hit */
  // in other words now store the simulated detector response
  if (dEsum > 0) {
    int nhit;
    s_FtofTruthHits_t* hits;
    s_FtofTruthExtras_t* extras;

    // getrow and getcolumn are both coded in hddsGeant3.F
    // see above for function getplane()
    
    int row = getrow_wrapper_();
    int column = getcolumn_wrapper_();
    
    // distance of hit from PMT north w.r.t. center and similar for PMT south
    // this means positive x points north. to get a right handed system y must
    // point vertically up as z is the beam axis.
    // plane==0 horizontal plane, plane==1 vertical plane
    //    float dist = xlocal[0];

    float dist = x[1]; // do not use local coordinate for x and y
    if (plane==1)
      dist = x[0];
    float dxnorth = BAR_LENGTH/2.-dist;
    float dxsouth = BAR_LENGTH/2.+dist;
    
    // calculate time at the PMT "normalized" to the center, so a hit in the 
    // center will have time "t" at both PMTs
    // the speed of signal travel is C_EFFECTIVE
    // propagte time to the end of the bar
    // column = 0 is a full paddle column ==1,2 is a half paddle

    float tnorth = t + dxnorth/C_EFFECTIVE;
    float tsouth = t + dxsouth/C_EFFECTIVE;
    
    // calculate energy seen by PM for this track step using attenuation factor
    float dEnorth = dEsum * exp(-dxnorth/ATTEN_LENGTH);
    float dEsouth = dEsum * exp(-dxsouth/ATTEN_LENGTH);

    if (plane==0){
      if (column==1){
	tnorth=0.;
	dEnorth=0.;
      }
      else if (column==2){	
	tsouth=0.;
	dEsouth=0.;
      }
    }
    else{
      if (column==2){
	tnorth=0.;
	dEnorth=0.;
      }
      else if (column==1){
	tsouth=0.;
	dEsouth=0.;
      }
    }


    int padl = row;
    if (row>44)
      padl = row-23;
 
    //int mark = (plane<<20) + (row<<10) + column;
    int mark = (plane<<20) + (padl<<10);// + column;
    void** twig = getTwig(&forwardTOFTree, mark);
    
    if (*twig == 0) { // this paddle has not been hit yet by any particle track 
                      // get space and store it

      s_ForwardTOF_t* tof = *twig = make_s_ForwardTOF();
      s_FtofCounters_t* counters = make_s_FtofCounters(1);
      counters->mult = 1;
      counters->in[0].plane = plane;
      //counters->in[0].bar = row;
      counters->in[0].bar = padl;
      hits = HDDM_NULL;

      // get space for the left/top or right/down PMT data for a total
      // of MAX_HITS possible hits in a single paddle
      // Note: column=0 means paddle read out on both ends, 
      //       column=1 means single-ended readout to north end
      //       column=2 means single-ended readout to south end

      if (column == 0 || column == 1 || column == 2) {
        counters->in[0].ftofTruthHits = hits = make_s_FtofTruthHits(MAX_HITS);
      }
      tof->ftofCounters = counters;
      counterCount++;

    } else { 

      // this paddle is already registered (was hit before)
      // get the hit list back
      s_ForwardTOF_t* tof = *twig;
      hits = tof->ftofCounters->in[0].ftofTruthHits;
    }
    
    if (hits != HDDM_NULL) {
      
      // loop over hits in this PM to find correct time slot, north end
      
      for (nhit = 0; nhit < hits->mult; nhit++)
      { 
        if (hits->in[nhit].end == 0 &&
            fabs(hits->in[nhit].t - t) < TWO_HIT_RESOL)
        {
          break;
        }
      }
      
      // this hit is within the time frame of a previous hit
      // combine the times of this weighted by the energy of the hit
      
      if (nhit < hits->mult) {         /* merge with former hit */
	float dEnew=hits->in[nhit].dE + dEnorth;
        hits->in[nhit].t = 
          (hits->in[nhit].t * hits->in[nhit].dE + tnorth * dEnorth) /dEnew;
	hits->in[nhit].dE=dEnew;
                
        // now add MC tracking information 
        // first get MC pointer of this paddle

        extras = hits->in[nhit].ftofTruthExtras;
        unsigned int nMChit = extras->mult;
        if (nMChit < MAX_HITS) {
          extras->in[nMChit].x = x[0];
          extras->in[nMChit].y = x[1];
          extras->in[nMChit].z = x[2];
          extras->in[nMChit].E = pin[3];
          extras->in[nMChit].px = pin[0]*pin[4];
          extras->in[nMChit].py = pin[1]*pin[4];
          extras->in[nMChit].pz = pin[2]*pin[4];
          extras->in[nMChit].ptype = ipart;
          extras->in[nMChit].itrack = gidGetId(track);
          extras->in[nMChit].dist = dist;
          extras->mult++;
        }
        
      }  else if (nhit < MAX_HITS){  // hit in new time window
        hits->in[nhit].t = tnorth;
        hits->in[nhit].dE = dEnorth;
        hits->in[nhit].end = 0;
        hits->mult++;

        // create memory for MC track hit information
        hits->in[nhit].ftofTruthExtras = 
                       extras = make_s_FtofTruthExtras(MAX_HITS);

        extras->in[0].x = x[0];
        extras->in[0].y = x[1];
        extras->in[0].z = x[2];
        extras->in[0].E = pin[3];
        extras->in[0].px = pin[0]*pin[4];
        extras->in[0].py = pin[1]*pin[4];
        extras->in[0].pz = pin[2]*pin[4];
        extras->in[0].ptype = ipart;
        extras->in[0].itrack = gidGetId(track);
        extras->in[0].dist = dist;
        extras->mult = 1;
        
      } else {
        fprintf(stderr,"HDGeant error in hitForwardTOF (file hitFTOF.c): ");
        fprintf(stderr,"max hit count %d exceeded, truncating!\n",MAX_HITS);
      }
      
      // loop over hits in this PM to find correct time slot, south end
      
      for (nhit = 0; nhit < hits->mult; nhit++)
      {
        if (hits->in[nhit].end == 1 &&
            fabs(hits->in[nhit].t - t) < TWO_HIT_RESOL)
        {
          break;
        }
      }
      
      // this hit is within the time frame of a previous hit
      // combine the times of this weighted by the energy of the hit
      
      if (nhit < hits->mult) {         /* merge with former hit */
	float dEnew=hits->in[nhit].dE + dEsouth;
        hits->in[nhit].t = 
          (hits->in[nhit].t * hits->in[nhit].dE + tsouth * dEsouth) / dEnew;
	hits->in[nhit].dE=dEnew;
        extras = hits->in[nhit].ftofTruthExtras;

        // now add MC tracking information 
        unsigned int nMChit = extras->mult;        
        if (nMChit < MAX_HITS) {
          extras->in[nMChit].x = x[0];
          extras->in[nMChit].y = x[1];
          extras->in[nMChit].z = x[2];
          extras->in[nMChit].E = pin[3];
          extras->in[nMChit].px = pin[0]*pin[4];
          extras->in[nMChit].py = pin[1]*pin[4];
          extras->in[nMChit].pz = pin[2]*pin[4];
          extras->in[nMChit].ptype = ipart;
          extras->in[nMChit].itrack = gidGetId(track);
          extras->in[nMChit].dist = dist;
          extras->mult++;
        }
        
      }  else if (nhit < MAX_HITS) {  // hit in new time window
        hits->in[nhit].t = tsouth;
        hits->in[nhit].dE = dEsouth;
        hits->in[nhit].end = 1;
        hits->mult++;

        // create memory space for MC track hit information
        hits->in[nhit].ftofTruthExtras = 
                        extras = make_s_FtofTruthExtras(MAX_HITS);
        extras->in[0].x = x[0];
        extras->in[0].y = x[1];
        extras->in[0].z = x[2];
        extras->in[0].E = pin[3];
        extras->in[0].px = pin[0]*pin[4];
        extras->in[0].py = pin[1]*pin[4];
        extras->in[0].pz = pin[2]*pin[4];
        extras->in[0].ptype = ipart;
        extras->in[0].itrack = gidGetId(track);
        extras->in[0].dist = dist;
        extras->mult = 1;
        
      } else {
        fprintf(stderr,"HDGeant error in hitForwardTOF (file hitFTOF.c): ");
        fprintf(stderr,"max hit count %d exceeded, truncating!\n",MAX_HITS);
      }
    }
  }
}

/* entry point from fortran */

void hitforwardtof_ (float* xin, float* xout,
                     float* pin, float* pout, float* dEsum,
                     int* track, int* stack, int* history, int* ipart)
{
   hitForwardTOF(xin,xout,pin,pout,*dEsum,*track,*stack,*history, *ipart);
}


/* pick and package the hits for shipping */
// this function is called by loadoutput() (coded in hddmOutput.c)
// which in turn is called by GUOUT at the end of each event

s_ForwardTOF_t* pickForwardTOF ()
{
  s_ForwardTOF_t* box;
  s_ForwardTOF_t* item;
  
  if ((counterCount == 0) && (pointCount == 0))
    {
      return HDDM_NULL;
    }
  
  box = make_s_ForwardTOF();
  box->ftofCounters = make_s_FtofCounters(counterCount);
  box->ftofTruthPoints = make_s_FtofTruthPoints(pointCount);
  
  while ((item = (s_ForwardTOF_t*) pickTwig(&forwardTOFTree))) {
    s_FtofCounters_t* counters = item->ftofCounters;
    int counter;
    s_FtofTruthPoints_t* points = item->ftofTruthPoints;
    int point;
    
    for (counter=0; counter < counters->mult; ++counter) {
      s_FtofTruthHits_t* hits = counters->in[counter].ftofTruthHits;
      
      /* compress out the hits below threshold */
      // cut off parameter is THRESH_MEV
      int iok,i;
      int mok=0;
      // loop over all hits in a counter for the left/up PMT
      for (iok=i=0; i < hits->mult; i++) {
        
        // check threshold
        if (hits->in[i].dE >= THRESH_MEV/1e3) {
          
          if (iok < i) {
            hits->in[iok] = hits->in[i];
          }
          ++mok;
          ++iok;
        }
      }
      
      if (iok) {
        hits->mult = iok;
      } else if (hits != HDDM_NULL){ // no hits left over for this PMT            
        counters->in[counter].ftofHits = HDDM_NULL;
        FREE(hits);
      }
      
      if (mok){ // total number of time independent FTOF hits in this counter
        int m = box->ftofCounters->mult++;
        // add the hit list of this counter to the list
        box->ftofCounters->in[m] = counters->in[counter];
      }
    } // end of loop over all counters
    
    if (counters != HDDM_NULL) {
      FREE(counters);
    }
    
    // keep also the MC generated primary track particles
    int last_track = -1;
    double last_t = 1e9;
    for (point=0; point < points->mult; ++point) {
       if (points->in[point].trackID->itrack > 0 &&
          (points->in[point].track != last_track ||
           fabs(points->in[point].t - last_t) > 0.1))
       {
          int m = box->ftofTruthPoints->mult++;
          box->ftofTruthPoints->in[m] = points->in[point];
          last_track = points->in[point].track;
          last_t = points->in[point].t;
       }
    }
    if (points != HDDM_NULL) {
      FREE(points);
    }
    FREE(item);
  }
  
  // reset the counters
  counterCount = pointCount = 0;

  // free the hit list memory used by this event
  if ((box->ftofCounters != HDDM_NULL) &&
      (box->ftofCounters->mult == 0)) {
    FREE(box->ftofCounters);
    box->ftofCounters = HDDM_NULL;
  }
  if ((box->ftofTruthPoints != HDDM_NULL) &&
      (box->ftofTruthPoints->mult == 0)) {
    FREE(box->ftofTruthPoints);
    box->ftofTruthPoints = HDDM_NULL;
  }
  if ((box->ftofCounters->mult == 0) &&
      (box->ftofTruthPoints->mult == 0)) {
    FREE(box);
    box = HDDM_NULL;
  }
  return box;
}
