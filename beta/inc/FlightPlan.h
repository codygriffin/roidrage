// Convenience operators for physics stuff
glm::vec2 operator- (const Entity& a, const Entity& b) {
  return a.get<Position>()->pos - b.get<Position>()->pos;
}

float operator ~(const Entity& a) {
  return a.get<Radius>()->mag;
}

// Creates adds a parking orbit to entity around center
// radius us just 1.14 * the radius of center
// TODO: barycentric orbits
void park(Entity& entity, Entity& center) {
  // direction of periapsis doesn't matter because we're circular
  entity.addOrReplace<Orbit>(glm::vec2(0.0f, ~center*1.5f), 0.0f, 0.0f, &entity, &center);

  Log::debug("new parking orbit for % around % - radius: %",
             entity.name(), center.name(),  ~center*1.5f);

  
  Entity* c = &center;
  Entity* e = &entity;
  entity.addOrReplace<Goal>([=](){
    float d = (~(*c)*1.5f  - glm::length((*c) - (*e)))/~(*c)*1.5f;
    return std::abs(d) < 0.30f;
  });
}

// Creates a transfer orbit that goes from a parking orbit
void transfer(Entity& entity, Entity& dest) {
  glm::vec2 apoapsis  = dest - entity;
  glm::vec2 periapsis = -glm::normalize(apoapsis) * ~dest * 1.5f;

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
    float d = (~(*cp)*1.5f  - glm::length((*cp) - (*ep)))/~(*cp)*1.5f;
    return std::abs(d) < 0.30f;
  });
}

void moveTo(Entity& entity, Entity& goal) {
  Log::debug("traversing from % to root", entity.name());
  auto current = &entity;
  std::deque<Entity*> src = {&entity};
  while (current) {
    if (current->has<Orbit>()) {
      auto next = current->get<Orbit>()->focus;
      src.push_back(next);
      Log::debug("adding %", next->name());
      current = next;
    } else {
      current = 0;
    }
  }

  Log::debug("traversing from % to root", goal.name());
  current = &goal;
  std::deque<Entity*> dst = {&goal};
  while (current) {
    if (current->has<Orbit>()) {
      auto next = current->get<Orbit>()->focus;
      Log::debug("adding %", next->name());
      dst.push_front(next);
      current = next;
    } else {
      current = 0;
    }
  }
  for (auto x : src) { Log::debug("src: %", x->name()); }
  for (auto y : dst) { Log::debug("dst: %", y->name()); }

  Log::debug("finding common ancestor");
  Entity* lca = 0;
  while (src.back() == dst.front()) {
    lca = src.back();
    src.pop_back();
    dst.pop_front();
  } 
  src.push_back(lca);
  Log::debug("ancestor = %", lca->name());
  
  std::deque<Entity*> entities(src.begin(), src.end());
  for (auto z : entities) { Log::debug("first: %", z->name()); }

  std::copy(dst.begin(), dst.end(), std::back_inserter(entities));
  for (auto z : entities) { Log::debug("appended: %", z->name()); }

  std::deque<std::function<void()>> plan;

  entities.pop_front();
  entities.pop_front();
  auto e = entities.begin();
  while (e != entities.end()) {
    Log::debug("transfer and park to %", (*e)->name());
    Entity* ptr1 = &entity;
    Entity* ptr2 = *e;
    plan.push_back([=](){ transfer(*ptr1, *ptr2); });
    plan.push_back([=](){ park(*ptr1, *ptr2); });
    e++;
  }

  Log::debug("committing to plan (% maneuvers)", plan.size());
  if (!plan.empty()) {
    entity.addOrReplace<FlightPlan>(plan);
    FlightPlan* plan = entity.get<FlightPlan>();
    Log::debug("initiating flight plan");
    plan->maneuvers.front()();
    plan->maneuvers.pop_front();
  }
}
