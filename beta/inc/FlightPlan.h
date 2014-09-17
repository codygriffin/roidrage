// Convenience operators for physics stuff
glm::vec2 operator- (const Entity& a, const Entity& b) {
  return a.get<Position>()->pos - b.get<Position>()->pos;
}

float operator ~(const Entity& a) {
  return a.get<Radius>()->mag;
}

float radiusOf(Entity* pEntity) {
  return pEntity->get<Radius>()->mag;
}

//------------------------------------------------------------------------------
// Some orbital some pre-programmed orbital maneuvers 


// Creates adds a parking orbit to entity around center
// radius us just 1.14 * the radius of center
// TODO: barycentric orbits
void park(Entity& entity, Entity& center, float r = 1.5f) {
  // direction of periapsis doesn't matter because we're circular
  entity.addOrReplace<Orbit>(glm::vec2(0.0f, ~center*r), 0.0f, 0.0f, &entity, &center);

  Log::debug("new parking orbit for % around % - radius: %",
             entity.name(), center.name(),  ~center*r);

  Entity* c = &center;
  Entity* e = &entity;
  
  entity.addOrReplace<Goal>([=](){
    float p = radiusOf(c)*r;
    float d = (p - glm::length((*c) - (*e)))/p;
    return std::abs(d) < 0.30f;
  });
}

void approach(Entity& entity, Entity& next) {
  Log::debug("% approaching %-orbit ",
             entity.name(), next.name());

  Entity* c = entity.get<Orbit>()->focus;
  Entity* e = &entity;
  Entity* n = &next;

  glm::vec2 periapsis = *c - *e;
  glm::vec2 apoapsis  = -glm::normalize(periapsis) * radiusOf(c) * 2.0f;

  // These don't change 
  const float rp = glm::length(periapsis);
  const float ra = glm::length(apoapsis);
  const float ec = std::abs((ra - rp) / (ra + rp));
  entity.addOrReplace<Orbit>(periapsis, ec, 0.0f, e, c);
  entity.addOrReplace<Goal>([=]() {
    auto  a = *e - *c;
    auto  b = *n - *c;
    auto  c = *e - *n;
    float ab = glm::length(a) * glm::length(b);
    float nr = 1.5f*~(*n);
    return std::abs(glm::length(c) - nr)/nr < 0.1 
        || std::abs(glm::dot(a,b)  - ab)/ab < 0.1;
  });
}

// Creates a transfer orbit that goes from a parking orbit
void transfer(Entity& entity, Entity& dest) {
  glm::vec2 apoapsis  = entity - dest;
  glm::vec2 periapsis = glm::normalize(apoapsis) * ~dest * 1.5f;

  // These don't change 
  const float rp = glm::length(periapsis);
  const float ra = glm::length(apoapsis);
  const float  e = std::abs((ra - rp) / (ra + rp));

  entity.addOrReplace<Orbit>(periapsis, e, 0.0f, &entity, &dest);
  Log::debug("new transfer orbit for % around % - Ra: % (%,%), Rp: % (%,%), e: %",
             entity.name(), dest.name(), 
             ra, apoapsis.x, apoapsis.y, rp, periapsis.x, periapsis.y, e);

  // Our goal is to get to the periapsis
  // TODO numerical error means we're not guaranteed to reach this goal
  // need a small correcting factor
  Entity* cp = &dest;
  Entity* ep = &entity;
  entity.addOrReplace<Goal>([=](){
    float r = radiusOf(cp)*1.5f;
    float d = (r - glm::length((*cp) - (*ep)))/r;
    return std::abs(d) < 0.30f;
  });
}

//------------------------------------------------------------------------------
// compose orbital maneuvers into a cohesive plan

void moveTo(Entity& entity, Entity& goal) {
  auto current = &entity;
  // Starting at our entity
  std::deque<Entity*> src = {&entity};
  // work our way up through the orbital tree
  while (current) {
    if (current->has<Orbit>()) {
      auto next = current->get<Orbit>()->focus;
      src.push_back(next);
      current = next;
    } else {
      current = 0;
    }
  }
  // Forget about this path

  // starting at the goal
  current = &goal;
  // work our way back up through the orbital tree
  std::deque<Entity*> dst = {&goal};
  while (current) {
    if (current->has<Orbit>()) {
      auto next = current->get<Orbit>()->focus;
      dst.push_front(next);
      current = next;
    } else {
      current = 0;
    }
  }

  // find the least common ancestor
  Entity* lca = 0;
  while (src.back() == dst.front()) {
    lca = src.back();
    src.pop_back();
    dst.pop_front();
  } 
  src.push_back(lca);
  
  // glue together our two lists (entity -> lcs and lcs -> goal)
  std::deque<Entity*> waypoints(src.begin(), src.end());
  std::copy(dst.begin(), dst.end(), std::back_inserter(waypoints));
  std::deque<std::function<void()>> plan;

  // ???
  waypoints.pop_front();
  waypoints.pop_front();
  auto w = waypoints.begin();

  // Go through our waypoints and build a flightplan
  while (w != waypoints.end()) {
    Log::debug("transfer and park to %", (*w)->name());
    Entity* ptr1 = &entity;
    Entity* ptr2 = *w;
    // to reach our next waypoint, we need to wait until
    // an approach
    // this is super naive - really we ought to do some optimization
    plan.push_back([=](){ approach(*ptr1, *ptr2); });
    // at the 'correct' time, we'll launch into a transfer orbit towards
    // the next waypoint.   This phase is over once we reach the 
    // periapsis of the transfer orbit
    plan.push_back([=](){ transfer(*ptr1, *ptr2); });
    // once we reach the next waypoint, we enter a circular parking orbit
    plan.push_back([=](){ park    (*ptr1, *ptr2); });
    w++;
  }

  Log::debug("committing to plan (% maneuvers)", plan.size());
  if (!plan.empty()) {
    // once we have the flightplan, we add th
    entity.addOrReplace<FlightPlan>(plan);
    FlightPlan* plan = entity.get<FlightPlan>();
    Log::debug("initiating flight plan");
    // execute the plan
    plan->maneuvers.front()();
    plan->maneuvers.pop_front();
  }
}
