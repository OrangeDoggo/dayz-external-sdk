External SDK for DayZ, enough to make esp and bullet tp, includes offsets for 1.20 although could be outdated as I last checked March 6th. Could be made cleaner and some things done better but good enough.

Usage:
```cpp
const auto world = world_t::instance();
const auto local_player = world_t::local_player();
const auto [near_entities, near_size] = world->near_entity_table();
for (auto i = 0; i < near_size; i++)
{
    const auto entity = near_entities->get(i);
    if (!entity || entity == local_player) continue;

    const auto type = entity->type();
    if (!type) continue;

    const auto name = type->config_name();
    if (!name.get()) continue;

    if (strcmp(name.get(), "dayzplayer") == 0)
        tmp.emplace_back<cached_entity_t>({ entity_type::PLAYER, entity });
    if (strcmp(name.get(), "dayzinfected") == 0)
        tmp.emplace_back<cached_entity_t>({ entity_type::INFECTED, entity });
}
```