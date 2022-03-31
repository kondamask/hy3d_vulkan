#include "hy3d_engine.h"

#include "camera.cpp"
//#include <intrin.h>

static_func void InitializeMemoryArena(memory_arena *arena, u8 *base, u64 size)
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

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
	engine_state *state = (engine_state *)memory->permanentMemory;
	update_data &update = state->updateData;
	platformAPI = memory->platformAPI_;

	if (!memory->isInitialized)
	{
		e.input = {};
		e.onResize = false;
		InitializeMemoryArena(&state->memoryArena,
			(u8 *)memory->permanentMemory + sizeof(engine_state),
			memory->permanentMemorySize - sizeof(engine_state));

		// NOTE: Everything is initialized here
		state->updateData.clearColor[0] = 0.1f;
		state->updateData.clearColor[1] = 0.3f;
		state->updateData.clearColor[2] = 0.6f;
		memory->nextStagingAddr = memory->stagingMemory;

		staged_resources sceneResources = {};
		sceneResources.nextWriteAddr = memory->nextStagingAddr;

		CreateScene(sceneResources, memory->objectsTransforms);
		platformAPI.PushStaged(sceneResources);

		memory->isInitialized = true;

		state->player.Initialize( { 0.0f, 2.0f, 3.0f }, { 0.0f, 0.0f, 1.0f }, VEC3_DOWN, 3.0f, 10.0f, 60.0f);

		e.input.mouse.cursorEnabled = false;

		state->time = 0.0f;
		e.frameStart = std::chrono::steady_clock::now();
	}

	// NOTE(heyyod): Frame time
	std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
	std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
	f32 dt = frameTime.count();
	state->time += dt;
	e.frameStart = frameEnd;

	// NOTE(heyyod): SETTINGS CONTROL
	update.clearColor = { 0.7f, 0.4f, 0.3f };

	{
		if (!e.input.keyboard.isPressed[KEY_ALT] && !e.input.keyboard.altWasUp)
			e.input.keyboard.altWasUp = true;
		if (e.input.keyboard.isPressed[KEY_ALT] && e.input.keyboard.altWasUp)
		{

			CHANGE_GRAPHICS_SETTINGS newSettings = CHANGE_NONE;
			MSAA_OPTIONS newMSAA = state->settings.msaa;

			if (e.input.keyboard.isPressed[KEY_ONE])
				newMSAA = MSAA_OFF;
			if (e.input.keyboard.isPressed[KEY_TWO])
				newMSAA = MSAA_2;
			if (e.input.keyboard.isPressed[KEY_THREE])
				newMSAA = MSAA_4;
			if (e.input.keyboard.isPressed[KEY_FOUR])
				newMSAA = MSAA_8;
			if (newMSAA != state->settings.msaa)
			{
				state->settings.msaa = newMSAA;
				newSettings |= CHANGE_MSAA;
				e.input.keyboard.altWasUp = false;
			}
			if (newSettings)
				platformAPI.ChangeGraphicsSettings(state->settings, newSettings);

			if (e.input.keyboard.isPressed[KEY_TILDE])
			{
				e.input.mouse.cursorEnabled = !e.input.mouse.cursorEnabled;
				e.input.keyboard.altWasUp = false;
			}
		}
	}

	// NOTE(heyyod): CAMERA CONTROL
	{
		camera &player = state->player;
		vec3 moveDir = {};
		if (e.input.keyboard.isPressed[KEY_W])
		{
			moveDir += player.dir;
		}
		if (e.input.keyboard.isPressed[KEY_S])
		{
			moveDir -= player.dir;
		}
		if (e.input.keyboard.isPressed[KEY_A])
		{
			moveDir += Cross(VEC3_DOWN, player.dir);
		}
		if (e.input.keyboard.isPressed[KEY_D])
		{
			moveDir += Cross(player.dir, VEC3_DOWN);
		}
		if (e.input.keyboard.isPressed[KEY_Q])
		{
			moveDir.Y -= 1.0f;
		}
		if (e.input.keyboard.isPressed[KEY_E])
		{
			moveDir.Y += 1.0f;
		}
		player.pos += player.speed * dt * Normalize(moveDir);

		if (!e.input.mouse.cursorEnabled)
		{
			f32 xOffset = -(e.input.mouse.newPos.X - e.input.mouse.lastPos.X) * dt;
			f32 yOffset = (e.input.mouse.lastPos.Y - e.input.mouse.newPos.Y) * dt; // reversed since y-coordinates range from bottom to top

			player.UpdateDir(xOffset, yOffset);
		}
		e.input.mouse.lastPos = e.input.mouse.newPos;
	}

	memory->cameraData->view = LookAt(state->player.pos, state->player.pos + state->player.dir, VEC3_DOWN);
	memory->cameraData->proj = Perspective(state->player.fov, e.windowWidth / (f32) e.windowHeight, 0.1f, 100.0f);
	memory->sceneData->ambientColor = { 0.5f, 0.5f, 0.65f, 0.0f };

	platformAPI.Draw(&state->updateData);

	// NOTE(heyyod): Update stuff from vulkan
	memory->cameraData = (camera_data *)update.newCameraBuffer;
	memory->sceneData = (scene_data *)update.newSceneBuffer;
}
