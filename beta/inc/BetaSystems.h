
Entity& createIndicator(Entity& e);
struct Selection {
  static void clear(){
    for (auto kv : selected) {
      game_.remove(kv.second->name());
    }
    selected.clear();
  }

  static void select(Entity& e) {
    auto item = selected.find(&e);
    if (item == selected.end()) {
      selected.emplace(std::make_pair(&e, &createIndicator(e)));
    }
  }
  static void unselect(Entity& e) {
    auto item = selected.find(&e);
    if (item != selected.end()) {
      game_.remove(item->second->name());
      selected.erase(item);
    }
  }

  static bool isSelected(Entity& e) {
    auto item = selected.find(&e);
    if (item != selected.end()) {
      return true;
    }
    return false;
  }
  
  static void toggle(Entity& e) {
    if (isSelected(e)) {
      unselect(e);
    } else {
      select(e);
    } 
  }

  static std::map<Entity*, Entity*> selected;
};
std::map<Entity*, Entity*> Selection::selected;

struct Picker {
  glm::vec2           start;
  glm::vec2           end;
  std::list<Entity*>  picked;
  std::list<Entity*>::iterator current;

  Picker() 
    : start ()
    , end ()
    , picked() 
    , current(picked.end()) {
  }

  std::list<Entity*>& query() {
    return picked;
  }

  void
  startBox(const glm::vec2& p) {
    start = p;
    end   = p;
  }

  void
  updateBox(const glm::vec2& p) {
    end = p;
  }

  Entity* cycle() {
    if (current != picked.end()) {
      auto e = *current;
      current++;
      return e;
    } else {
      current = picked.begin();
      return *current;
    }
  }

  void reset() {
    picked.clear();
  }

  bool isRange() {
    return start != end;
  }

  bool intersects(glm::vec2 cp, float rr, glm::vec2 rp1, glm::vec2 rp2)
  {
     auto x1 = std::min(rp1.x, rp2.x);
     auto y1 = std::min(rp1.y, rp2.y);
     auto x2 = std::max(rp1.x, rp2.x);
     auto y2 = std::max(rp1.y, rp2.y);
     float closestX = (cp.x < x1 ? x1 : (cp.x > x2 ? x2 : cp.x));
     float closestY = (cp.y < y1 ? y1 : (cp.y > y2 ? y2 : cp.y));
     float dx = closestX - cp.x;
     float dy = closestY - cp.y;

     return ( dx * dx + dy * dy ) <= rr * rr;
  }

  void 
  testPickable(Pickable* s, Position* p, Radius* r) {
    static const float paddingFactor = 2.0f;  

    auto& entity      = game_.entity(s->entity);

    if (isRange()) {
      if (intersects(p->pos, r->mag, start, end)) { 
        picked.push_back(&entity);
      } 
    } else {
      auto displacement = p->pos - start;
      if (paddingFactor*r->mag > glm::length(displacement)) { 
        picked.push_back(&entity);
      } 
    }
  }

  void 
  testHillSphere(Position* p, HillSphere* r) {
    // TODO abort system exec()
    auto displacement = p->pos - start;

    auto& entity = game_.entity(r->entity);

    if (r->mag > glm::length(displacement)) { 
      picked.push_back(&entity);
    }
  }
};

struct Acceleration {
  void reset(Position* pPos) {
    pPos->acc  = glm::vec2(0.0f);
    pPos->aacc = 0.0f;
  }

  //------------------------------------------------------------------------------

  void update(Position* p1, Mass* m1) {
    current_ = std::make_tuple(p1, m1);
    game_.exec(*this, &Acceleration::update2);
  }

  //------------------------------------------------------------------------------

  std::tuple<Position*, Mass*> current_;
  void update2(Position* p2, Mass* m2) {
    Position* p1; 
    Mass*     m1; 
    std::tie(p1, m1) = current_;
    if (p1 == p2) return;

    glm::vec2 diff = p2->pos - p1->pos;
    float r = glm::length(diff);

    // Clamp to sane values
    if (r <  10.0f)          return;
    if (r >  1500.0f)        return;
    if (m1->mag < 0.00001f)  return;

    const float mr2 = m1->mag*r*r;
    glm::normalize(diff);

    glm::vec2 acc  = diff *   Mass::unit*m1->mag*m2->mag / mr2;

    p1->acc += acc;
    if (p1->acc.x != p1->acc.x && p1->acc.y != p1->acc.y) {
      Log::error("Acceleration is NaN after update");
      p1->acc = glm::vec2(0.0f, 0.0f);
    }
  }
};

struct CollisionDetector {
  void update(Handler<events::Collision>* c1, Position* p1, Radius* r1) {
    current_ = std::make_tuple(c1, p1, r1);
    game_.exec(*this, &CollisionDetector::update2);
  }

  //------------------------------------------------------------------------------

  void update2(Handler<events::Collision>* c2, Position* p2, Radius* r2) {
    static const float paddingFactor = 0.8f;  

    Handler<events::Collision>* c1; 
    Position* p1; 
    Radius*   r1; 
    std::tie(c1, p1, r1) = current_;
    if (p1 == p2) return;

    // Boundary is the minimum allowable distance 
    // between colliding entities
    auto boundary = r1->mag + r2->mag;

    // Displacement is the vector pointing from this entity to the 
    // other entity, with a length corresponding to the distance
    auto displacement = p1->pos - p2->pos;

    if (boundary * paddingFactor > glm::length(displacement)) { 
      // Really, we only care about overlap in the collision
      displacement = glm::normalize(displacement) * 
                    (boundary - glm::length(displacement));
      //Collision collision(displacement, boundary);
      c2->handler(events::Collision());
      //b2->onCollideWith(b1, &collision);
    }
  }

  std::tuple<Handler<events::Collision>*, Position*, Radius*> current_;
};

//------------------------------------------------------------------------------

void 
updateTime(Time* time) {
  float current = glfwGetTime();
  time->vel = current - time->pos;
  time->pos = current; 
}

//------------------------------------------------------------------------------

void updatePosition(Time* time, Position* pos) {
  //TODO better integration (RK4?)
  if (std::isnan(pos->acc.x) || std::isnan(pos->acc.y)) {
    throw std::runtime_error("NaN acceleration!");
  }

  pos->vel     += pos->acc     * float(time->vel);

  if (std::isnan(pos->vel.x) || std::isnan(pos->vel.y)) {
    throw std::runtime_error("NaN velocity!");
  }

  pos->pos     += pos->vel     * float(time->vel);

  if (std::isnan(pos->pos.x) || std::isnan(pos->pos.y)) {
    throw std::runtime_error("NaN position!");
  }
}

void track(Track<Position>* t, Position* p) {
  p->pos     = t->value->pos + glm::vec2(-100.0f, -100.0f);
}


void updateOrientation(Time* time, Orientation* o) {
  o->avel    += o->aacc    * float(time->vel);
  o->apos    += o->avel    * float(time->vel);

  // Wrap 
  if (o->apos >  360.0f) {
    o->apos = 0.0f;
  }
  if (o->apos < -360.0f) {
    o->apos = 0.0f;
  }

  // Clamp 
  o->avel = std::min(o->avel,  30.0f);
  o->avel = std::max(o->avel, -30.0f);
  o->aacc = std::min(o->aacc,  0.004f);
  o->aacc = std::max(o->aacc, -0.004f);
}


//------------------------------------------------------------------------------

struct Transformations {
  void 
  transform1(Transform* x, Position* p) {
    transforms_.emplace(std::make_pair(x->depth(), [=]() {
      x->transform = glm::mat4();

      // Apply parent transformations
      // TODO order of this index matters now (parent/child dependency between transforms)
      if (x->parent) { 
        x->transform = x->parent->transform;   
      }

      // Set our position, rotation and scale according
      // to the position, radius and angular position.
      x->transform = glm::translate(
        x->transform, 
        glm::vec3(p->pos.x, p->pos.y, 0.0f)
      );
    }));
  }

  void 
  transform2(Transform* x, Orientation* o) {
    transforms_.emplace(std::make_pair(x->depth(), [=]() {
      x->transform = glm::rotate(
        x->transform, 
        glm::radians(o->apos),
        glm::vec3(0.0f,0.0f,1.0f)
      );
    }));
  }

  void 
  transform3(Transform* x, Radius* r) {
    x->transform = glm::scale(
      x->transform, 
      glm::vec3(r->mag)
    );
  }
  
  void
  exec() {
    for (auto kv : transforms_) {
      kv.second();
    }
    transforms_.clear();
  }

  std::multimap<unsigned, std::function<void (void)>> transforms_;
};

//------------------------------------------------------------------------------

void
flightGoals(Goal* g, FlightPlan* p) {
  if (g->criteria()) {
    if (!p->maneuvers.empty()) {
      Log::debug("goal met - next maneuver");
      p->maneuvers.front()();
      p->maneuvers.pop_front();
    }
  }
}

//------------------------------------------------------------------------------

void
flightControl(Position* p, Mass* m, Orbit* o, Orientation *b) {
  auto d     = p->pos - o->focus->get<Position>()->pos;
  auto dir   = glm::normalize(d);
  auto r     = glm::length(d);
  auto e     = o->eccentricity;
  auto ra = glm::length(o->periapsis) * (1.0f + e)/(1.0f - e);
  auto rp    = o->periapsis + o->focus->get<Position>()->pos;

  const float pi = 3.14159f;


  float u     = Mass::unit*o->focus->get<Mass>()->mag;
  float y     = glm::dot(dir, glm::normalize(o->periapsis));
  if (y > 1.0f) y =  1.0f;
  if (y <-1.0f) y = -1.0f;
  float theta = acos(y);
  float l     = dir.x*o->periapsis.y - dir.y*o->periapsis.x;
  if (l > 0.0f) {
    theta = 2.0f * pi - theta;
  }
  if (theta > 2.0f * pi) theta -= 2.0f * pi;
  if (theta < 0.0f)      theta += 2.0f * pi;

  auto k = sqrtf(u / glm::length(rp) * (1.0f + e));

  auto  normalDirection = glm::normalize(glm::vec2(d.y, -d.x));
  auto  radialDirection = glm::normalize(d);

  p->vel = radialDirection * k * e * sinf(theta)
         + normalDirection * k * (1.0f + e * cosf(theta))
         + o->focus->get<Position>()->vel;

  b->apos = atan2(dir.y, dir.x) * 360.0f/(2.0f*pi) + 90.0f;

  if (glm::length(p->vel) > 1e13) {
    throw std::runtime_error("runaway!");
  }
}

//------------------------------------------------------------------------------

void renderSolid(Transform* x, Color* c, GlProgram* p, GlVbo* v) {
  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("mModel",      matView * x->transform);
  p->program.uniform("vColor",      c->color);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

void renderTextured(Transform* x, GlTexture* t, GlProgram* p, GlVbo* v) {
  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("mModel",      matView * x->transform);
  p->program.uniform("uTexture",    t->texture);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

void 
renderText(String* s, Transform* x, Color* c, GlTexture* t, GlProgram* p, GlVbo* v) {
  // TODO parameterize
  const float r       = s->size;
  const float offsetx = r;
  const float offsety = r;
  const float padx    = 0.0f;
  const float pady    = r/2.0f;
  const std::string& text    = s->str;

  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;

  matView *= x->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("uTexture",    t->texture);
  p->program.uniform("vColor",      c->color);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));

  float x_ = 0; 
  float y_ = 0;
  for (size_t i  = 0; i < text.length(); i++) {
    if (text[i] == '\n') {
      y_ += r + pady; 
      x_ = 0;
    } else {
      int nx = text[i] % 16;
      int ny = text[i] / 16;

      // character offset within texture
      auto translated = glm::translate(matView, glm::vec3(x_, y_, 0.0f));
      auto scaled     = glm::scale(translated, glm::vec3(r));

      p->program.uniform  ("offsetX", (float)nx);
      p->program.uniform  ("offsetY", (float)ny);
      p->program.uniform  ("mModel", scaled);

      p->program.execute  (0, 4);

      x_ += r + padx;
    }
  }
}

