#include <set>
#include <vector>
#include <list>
#include <functional>
#include <thread>

#include "TimeMs.h"
#include "Params.h"
#include "Log.h"

// Structure
#include "Corvid.h"
#include "Relation.h"
#include "Population.h"

// Entities
#include "PewPew.h"
#include "Ship.h"
#include "Bonus.h"
#include "Roid.h"
#include "Text.h"
#include "Background.h"

using namespace pronghorn;
using namespace roidrage;

#include "RenderState.h"
#include "AudioState.h"
#include "OrthoCamera.h"
#include "RoidRage.h"
#include "RoidRageMenu.h"
#include "RoidRageGame.h"

static void setupGame(int w, int h, float d) {
  Log::info("********************************************************************************");
  pRoidRage = new RoidRageMachine(w, h, d);
  pRoidRage->transition<RoidRageMenu>(true);
  //pRoidRage->transition<RoidRageGame>(true);
  Log::info("RoidRageInitialized (w=%, h=%, d=%)", w, h, d);
  Log::info("********************************************************************************");
}

static void destroy() {
  pRoidRage->pMusicPlayer->stop();
}

static void touchGame(int action, int index, int count, int* p, int* x, int* y, int* s) {
  pRoidRage->dispatch(Touch(action, index, count, p, x, y));
}

static void zoomGame(float zoom) {
  RenderState::pCam_->setZoom(zoom);
}

static void back() {
  pRoidRage->dispatch(AndroidBack());
}

static void runGame() {
  pRoidRage->dispatch(Tick());
}
