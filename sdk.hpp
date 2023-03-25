class entity_t;

struct matrix_t {
	float m[16];
};

class type_t
{
public:
	std::unique_ptr<char[]> config_name()
	{
		return memory->read_arma_string(memory->read(this + offsets::entity_type::config_name));
	}

	std::unique_ptr<char[]> clean_name()
	{
		return memory->read_arma_string(memory->read(this + offsets::entity_type::clean_name));
	}

	std::unique_ptr<char[]> model_name()
	{
		return memory->read_arma_string(memory->read(this + offsets::entity_type::model_name));
	}
};

class visual_state_t
{
public:
	vec3_t position()
	{
		return memory->read<vec3_t>(this + offsets::visual_state::position);
	}

	void position(vec3_t value)
	{
		memory->write<vec3_t>(this + offsets::visual_state::position, value);
	}

	matrix_t matrix()
	{
		return memory->read<matrix_t>(this + offsets::visual_state::matrix);
	}
};

class inventory_t
{
public:
	entity_t* item_in_hands()
	{
		return memory->read<entity_t*>(this + offsets::inventory::item_in_hands);
	}
};

class anim_t
{
public:
	vec3_t get_matrix_by_idx(int idx)
	{
		return memory->read<vec3_t>(memory->read(this + offsets::anim_class::matrixes) + 0x54 + idx * sizeof(matrix_t));
	}
};

class skeleton_t
{
public:
	anim_t* anim()
	{
		return memory->read<anim_t*>(this + offsets::skeleton::anim_class);
	}
};

class entity_t
{
public:
	uint32_t network_id()
	{
		return memory->read<uint32_t>(this + offsets::entity::network_id);
	}

	type_t* type()
	{
		return memory->read<type_t*>(this + offsets::entity::entity_type);
	}

	visual_state_t* visual_state()
	{
		return memory->read<visual_state_t*>(this + offsets::entity::visual_state);
	}

	inventory_t* inventory()
	{
		return memory->read<inventory_t*>(this + offsets::entity::inventory);
	}

	bool is_dead()
	{
		return memory->read<uint8_t>(this + offsets::entity::is_dead) == 1;
	}

	bool is_player()
	{
		const auto type = this->type();
		if (!type) return true;

		const auto name = type->config_name();
		if (!name) return true;

		return strcmp(name.get(), "dayzplayer") == 0;
	}

	skeleton_t* skeleton()
	{
		return memory->read<skeleton_t*>(this + (this->is_player() ? offsets::entity::player_skeleton : offsets::entity::zombie_skeleton));
	}

	vec3_t get_bone_position(int idx)
	{
		const auto skeleton = this->skeleton();
		if (!skeleton) return vec3_t();

		const auto anim = skeleton->anim();
		if (!anim) return vec3_t();

		const auto matrix_a = this->visual_state()->matrix();
		const auto matrix_b = anim->get_matrix_by_idx(idx);

		return vec3_t(
			(matrix_a.m[0] * matrix_b.x) + (matrix_a.m[3] * matrix_b.y) + (matrix_a.m[6] * matrix_b.z) + matrix_a.m[9],
			(matrix_a.m[1] * matrix_b.x) + (matrix_a.m[4] * matrix_b.y) + (matrix_a.m[7] * matrix_b.z) + matrix_a.m[10],
			(matrix_a.m[2] * matrix_b.x) + (matrix_a.m[5] * matrix_b.y) + (matrix_a.m[8] * matrix_b.z) + matrix_a.m[11]
		);
	}
};

class table_t
{
public:
	entity_t* get(uint32_t idx)
	{
		return memory->read<entity_t*>(this + (idx * sizeof(void*)));
	}
};

class weird_table_t
{
public:
	uint32_t flag(uint32_t idx)
	{
		return memory->read<uint32_t>(this + (idx * 0x18));
	}

	entity_t* get(uint32_t idx)
	{
		return memory->read<entity_t*>(this + ((idx * 0x18) + 0x8));
	}
};

class camera_t
{
public:
	vec3_t view_translation()
	{
		return memory->read<vec3_t>(this + offsets::camera::view_translation);
	}

	vec3_t view_right()
	{
		return memory->read<vec3_t>(this + offsets::camera::view_right);
	}

	vec3_t view_up()
	{
		return memory->read<vec3_t>(this + offsets::camera::view_up);
	}

	vec3_t view_forward()
	{
		return memory->read<vec3_t>(this + offsets::camera::view_forward);
	}

	vec3_t viewport()
	{
		return memory->read<vec3_t>(this + offsets::camera::viewport);
	}

	vec3_t projection_d1()
	{
		return memory->read<vec3_t>(this + offsets::camera::projection_d1);
	}

	vec3_t projection_d2()
	{
		return memory->read<vec3_t>(this + offsets::camera::projection_d2);
	}

	bool world_to_screen(vec3_t world, vec2_t& out)
	{
		if (!this) return false;

		vec3_t temp = world - this->view_translation();
		vec3_t vp = this->viewport();

		vec3_t view = vec3_t(
			temp.dot(this->view_right()),
			temp.dot(this->view_up()),
			temp.dot(this->view_forward())
		);

		if (view.z < 0.1f)
			return false;

		out = vec2_t(
			vp.x * (1 + (view.x / this->projection_d1().x / view.z)), 
			vp.y * (1 - (view.y / this->projection_d2().y / view.z))
		);

		return true;
	}
};

class player_identity_t
{
public:
	uint32_t network_id()
	{
		return memory->read<uint32_t>(this + offsets::player_identity::network_id);
	}

	std::unique_ptr<char[]> name()
	{
		return memory->read_arma_string(memory->read(this + offsets::player_identity::player_name));
	}
};

class scoreboard_t
{
public:
	player_identity_t* get(uint32_t idx)
	{
		return memory->read<player_identity_t*>(this + offsets::scoreboard::player_identities + (idx * sizeof(void*)));
	}
};

class network_client_t
{
public:
	uint32_t identity_count()
	{
		return memory->read<uint32_t>(this + offsets::network_client::identity_count);
	}

	scoreboard_t* scoreboard()
	{
		return memory->read<scoreboard_t*>(this + offsets::network_client::scoreboard);
	}

	std::unique_ptr<char[]> player_name(entity_t* entity)
	{
		if (!this) return nullptr;

		const auto network_id = entity->network_id();
		const auto scoreboard = this->scoreboard();
		for (int i = 0; i < this->identity_count(); i++)
		{
			const auto identity = scoreboard->get(i);
			if (!identity) continue;

			if (identity->network_id() == network_id)
				return std::move(identity->name());
		}

		return nullptr;
	}
};

class network_manager_t
{
public:
	static network_manager_t* instance()
	{
		return memory->read<network_manager_t*>(memory->base_address + offsets::network_manager::instance);
	}

	static network_client_t* network_client()
	{
		return memory->read<network_client_t*>(memory->base_address + offsets::network_manager::instance + offsets::network_manager::network_client);
	}
};

class world_t
{
public:
	static world_t* instance()
	{
		return memory->read<world_t*>(memory->base_address + offsets::world::instance);
	}
	
	std::tuple<table_t*, std::uint32_t> near_entity_table()
	{
		return std::make_tuple(memory->read<table_t*>(this + offsets::world::near_entity_table), memory->read<uint32_t>(this + offsets::world::near_entity_table_size));
	}

	std::tuple<table_t*, std::uint32_t> far_entity_table()
	{
		return std::make_tuple(memory->read<table_t*>(this + offsets::world::far_entity_table), memory->read<uint32_t>(this + offsets::world::far_entity_table_size));
	}

	std::tuple<weird_table_t*, std::uint32_t> slow_entity_table()
	{
		return std::make_tuple(memory->read<weird_table_t*>(this + offsets::world::slow_entity_table), memory->read<uint32_t>(this + offsets::world::slow_entity_table_size) * 3);
	}

	std::tuple<weird_table_t*, std::uint32_t> item_table()
	{
		return std::make_tuple(memory->read<weird_table_t*>(this + offsets::world::item_table), memory->read<uint32_t>(this + offsets::world::item_table_size) * 3);
	}

	std::tuple<table_t*, std::uint32_t> bullet_table()
	{
		return std::make_tuple(memory->read<table_t*>(this + offsets::world::bullet_table), memory->read<uint32_t>(this + offsets::world::bullet_table_size));
	}

	camera_t* camera()
	{
		return memory->read<camera_t*>(this + offsets::world::camera);
	}

	entity_t* local_player()
	{
		return memory->read<entity_t*>(this, { offsets::world::base_entity, 0x8 }) - 0xA8;
	}
};