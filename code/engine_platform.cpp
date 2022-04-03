#include "engine_platform.h"

#include "camera.cpp"
//#include <intrin.h>

inline static_func void InitializeMemoryArena(memory_arena *arena, u8 *base, u64 size)
{
	arena->base = base;
	arena->size = size;
	arena->used = 0;
}

#define ReserveStructMemory(arena, type) (type *)ReserveMemory(arena, sizeof(type))
#define ReserveArrayMemory(arena, count, type) (type *)ReserveMemory(arena, (count) * sizeof(type))

static_func void *ReserveMemory(memory_arena *arena, size_t size)
{
	Assert(arena->used + size <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += size;
	return result;
}

inline static_func void InitializeEngine(engine_context *engine)
{
	engine->input = {};
	engine->onResize = false;
	InitializeMemoryArena(&engine->state->memoryArena, (u8 *)engine->memory.permanentMemory + sizeof(engine_state), engine->memory.permanentMemorySize - sizeof(engine_state));

	// NOTE: Everything is initialized here
	engine->state->updateData.clearColor[0] = 0.1f;
	engine->state->updateData.clearColor[1] = 0.3f;
	engine->state->updateData.clearColor[2] = 0.6f;

	engine->memory.nextStagingAddr = engine->memory.stagingMemory;
	engine->memory.sceneData->ambientColor = { 0.5f, 0.5f, 0.65f, 0.0f };

	staged_resources sceneResources = {};
	sceneResources.nextWriteAddr = engine->memory.nextStagingAddr;

	CreateScene(sceneResources, engine->memory.objectsTransforms);
	platformAPI.PushStaged(sceneResources);

	engine->memory.isInitialized = true;

	engine->state->player.Initialize( { 0.0f, 2.0f, -5.0f }, { 0.0f, 0.0f, 1.0f }, VEC3_UP, 3.0f, 0.1f, 60.0f);

	engine->input.mouse.cursorEnabled = true;
	engine->input.mouse.firstMove = true;

	engine->state->time = 0.0f;
	engine->frameStart = std::chrono::steady_clock::now();
}

inline static_func void ProcessInput(engine_context *engine)
{
	// NOTE(heyyod): SETTINGS CONTROL
	engine->state->updateData.clearColor = { 0.7f, 0.4f, 0.3f };

	{
		if (!engine->input.keyboard.isPressed[KEY_ALT] && !engine->input.keyboard.altWasUp)
			engine->input.keyboard.altWasUp = true;
		if (engine->input.keyboard.isPressed[KEY_ALT] && engine->input.keyboard.altWasUp)
		{

			CHANGE_GRAPHICS_SETTINGS newSettings = CHANGE_NONE;
			MSAA_OPTIONS newMSAA = engine->state->settings.msaa;

			if (engine->input.keyboard.isPressed[KEY_ONE])
				newMSAA = MSAA_OFF;
			if (engine->input.keyboard.isPressed[KEY_TWO])
				newMSAA = MSAA_2;
			if (engine->input.keyboard.isPressed[KEY_THREE])
				newMSAA = MSAA_4;
			if (engine->input.keyboard.isPressed[KEY_FOUR])
				newMSAA = MSAA_8;
			if (newMSAA != engine->state->settings.msaa)
			{
				engine->state->settings.msaa = newMSAA;
				newSettings |= CHANGE_MSAA;
				engine->input.keyboard.altWasUp = false;
			}
			if (newSettings)
				platformAPI.ChangeGraphicsSettings(engine->state->settings, newSettings);

			if (engine->input.keyboard.isPressed[KEY_TILDE])
			{
				engine->input.mouse.cursorEnabled = !engine->input.mouse.cursorEnabled;
				engine->input.mouse.firstMove = true;
				engine->input.keyboard.altWasUp = false;
			}
		}
	}

	// NOTE(heyyod): CAMERA CONTROL
	// BUG: something is wrong with the up vector.
	// Using VEC3_UP renders everything upside down
	{
		camera &player = engine->state->player;
		vec3 dPos = {};
		if (engine->input.keyboard.isPressed[KEY_W])
			dPos += player.dir;
		if (engine->input.keyboard.isPressed[KEY_S])
			dPos -= player.dir;
		if (engine->input.keyboard.isPressed[KEY_A])
			dPos += Cross(VEC3_DOWN, player.dir);
		if (engine->input.keyboard.isPressed[KEY_D])
			dPos += Cross(player.dir, VEC3_DOWN);
		if (engine->input.keyboard.isPressed[KEY_Q])
			dPos.Y -= 1.0f;
		if (engine->input.keyboard.isPressed[KEY_E])
			dPos.Y += 1.0f;
		dPos *= engine->state->dt;

		vec2 dLook = {};
		if (engine->input.mouse.rightIsPressed)
		{
			engine->input.mouse.cursorEnabled = false;
			if (engine->input.mouse.firstMove)
			{
				engine->input.mouse.lastPos = engine->input.mouse.newPos;
				engine->input.mouse.firstMove = false;
			}

			dLook = {
				engine->input.mouse.lastPos.X - engine->input.mouse.newPos.X,
				engine->input.mouse.lastPos.Y - engine->input.mouse.newPos.Y
			} * engine->state->dt;
		}
		else
		{
			engine->input.mouse.cursorEnabled = true;
		}
		player.Update(dPos, dLook);

		engine->input.mouse.lastPos = engine->input.mouse.newPos;
	}
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
	engine_memory *memory = &engine->memory;
	engine_state *state = engine->state = (engine_state *)memory->permanentMemory;
	platformAPI = memory->platformAPI_;

	if (!memory->isInitialized)
	{
		InitializeEngine(engine);
	}

	// NOTE(heyyod): Frame time
	// TODO: Use cpu timing instead
	std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
	std::chrono::duration<f32> frameTime = frameEnd - engine->frameStart;
	state->dt = frameTime.count();
	state->time += state->dt;
	engine->frameStart = frameEnd;

	ProcessInput(engine);

	memory->cameraData->view = LookAt(state->player.pos, state->player.pos + state->player.dir, VEC3_DOWN);
	memory->cameraData->proj = Perspective(state->player.fov, engine->windowWidth / (f32) engine->windowHeight, 0.1f, 100.0f);

	platformAPI.Draw(&state->updateData);

	// NOTE(heyyod): Update stuff from vulkan
	memory->cameraData = (camera_data *)state->updateData.newCameraBuffer;
	memory->sceneData = (scene_data *)state->updateData.newSceneBuffer;
}
