#include "engine_platform.h"

#include "camera.cpp"
#include "renderer_platform.cpp"

#define DEFAULT_SHADER_FILEPATH ".\\assets\\shaders\\default.vert.spv"

inline static_func void MemoryArenaInitialize(memory_arena *arena, u8 *base, u64 size)
{
	arena->base = base;
	arena->size = size;
	arena->used = 0;
}

#define ReserveStructMemory(arena, type) (type *)MemoryArenaReserve(arena, sizeof(type))
#define ReserveArrayMemory(arena, count, type) (type *)MemoryArenaReserve(arena, (count) * sizeof(type))

static_func void *MemoryArenaReserve(memory_arena *arena, size_t size)
{
	Assert(arena->used + size <= arena->size);
	void *result = arena->base + arena->used;
	arena->used += size;
	return result;
}

extern "C" FUNC_ENGINE_INITIALIZE(EngineInitialize)
{
	engine->input = {};
	engine->onResize = false;

	engine_memory *memory = &engine->memory;
	engine_state *state = engine->state = (engine_state *)memory->permanentMemory;
	platformAPI = memory->platformAPI_;

	MemoryArenaInitialize(&state->memoryArena, (u8 *)memory->permanentMemory + sizeof(engine_state), memory->permanentMemorySize - sizeof(engine_state));

	// NOTE: Everything is initialized here
	state->renderPacket.clearColor = {0.0f, 0.4f, 0.3f};

	if (!RendererInitialize(RENDERER_GRAPHICS_API_VULKAN, engine))
	{
		DebugPrintFunctionResult(false);
		Assert(0);
	}

	platformAPI.GetFileWriteTime(DEFAULT_SHADER_FILEPATH, &engine->shadersWriteTime);
	engine->memory.stagingMemory = vulkanContext->stagingBuffer.data;

	// TODO(heyyod): This assumes that the first image we aquire in vulkan will always have index 0
	// Mayby bad
	engine->memory.cameraData = (camera_data *)vulkanContext->frameData[0].cameraBuffer.data;
	engine->memory.sceneData = (scene_data *)vulkanContext->frameData[0].sceneBuffer.data;
	engine->memory.objectsTransforms = (object_transform *)vulkanContext->staticTransformsBuffer.data;
	engine->memory.nextStagingAddr = engine->memory.stagingMemory;
	engine->memory.sceneData->ambientColor = {0.5f, 0.5f, 0.65f, 0.0f};


	staged_resources sceneResources = {};
	sceneResources.nextWriteAddr = engine->memory.nextStagingAddr;

	// Make initial scene
	CreateScene(sceneResources, engine->memory.objectsTransforms);
	engine->renderer.Upload(&sceneResources);
 

	engine->memory.isInitialized = true;

	CameraInitialize(engine->state->player, {0.0f, 2.0f, -5.0f}, {0.0f, 0.0f, -1.0f}, VEC3_UP, 4.0f, 2.0f, 60.0f);

	engine->input.mouse.cursorEnabled = true;
	engine->input.mouse.firstMove = true;

	engine->state->time = 0.0f;
	engine->frameStart = std::chrono::steady_clock::now();
}

inline static_func void EngineProcessInput(engine_context *engine)
{
	// NOTE(heyyod): SETTINGS CONTROL
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
			engine->renderer.ChangeGraphicsSettings(engine->state->settings, newSettings);

		if (engine->input.keyboard.isPressed[KEY_TILDE])
		{
			engine->input.mouse.cursorEnabled = !engine->input.mouse.cursorEnabled;
			engine->input.mouse.firstMove = true;
			engine->input.keyboard.altWasUp = false;
		}
	}

	// NOTE(heyyod): CAMERA CONTROL
	if (engine->input.mouse.rightIsPressed)
	{
		camera &player = engine->state->player;
		vec3 dPos = {};
		vec2 dLook = {};

		if (engine->input.keyboard.isPressed[KEY_W])
			dPos += player.dir;
		if (engine->input.keyboard.isPressed[KEY_S])
			dPos -= player.dir;
		if (engine->input.keyboard.isPressed[KEY_A])
			dPos += Cross(VEC3_UP, player.dir);
		if (engine->input.keyboard.isPressed[KEY_D])
			dPos += Cross(player.dir, VEC3_UP);
		if (engine->input.keyboard.isPressed[KEY_Q])
			dPos.Y -= 1.0f;
		if (engine->input.keyboard.isPressed[KEY_E])
			dPos.Y += 1.0f;
		dPos *= engine->state->renderPacket.dt;

		if (engine->input.keyboard.isPressed[KEY_SHIFT])
			dPos *= 6.0f; // SPEED BOOST

		engine->input.mouse.cursorEnabled = false;
		if (engine->input.mouse.firstMove)
		{
			engine->input.mouse.lastPos = engine->input.mouse.newPos;
			engine->input.mouse.firstMove = false;
		}

		dLook = {engine->input.mouse.newPos.X - engine->input.mouse.lastPos.X,
				 engine->input.mouse.lastPos.Y - engine->input.mouse.newPos.Y};
		dLook *= engine->state->renderPacket.dt;

		CameraUpdate(player, dPos, dLook);
	}
	else
	{
		engine->input.mouse.cursorEnabled = true;
	}

	engine->input.mouse.lastPos = engine->input.mouse.newPos;
}

extern "C" FUNC_ENGINE_UPDATE_AND_RENDER(EngineUpdateAndRender)
{
	Assert(engine->memory.isInitialized);

	engine_memory *memory = &engine->memory;
	engine_state *state = engine->state = (engine_state *)memory->permanentMemory;
	platformAPI = memory->platformAPI_;

	if (!engine->renderer.canRender)
		return;

	if (engine->reloaded)
	{
		RendererOnEngineReload(engine);
		engine->reloaded = false;
	}

	// Check if shader have been updated
	if (platformAPI.WasFileUpdated(DEFAULT_SHADER_FILEPATH, &engine->shadersWriteTime))
	{
		engine->renderer.OnShaderReload();
	}

	if (engine->onResize)
	{
		engine->renderer.OnResize(&engine->renderer);

		engine->onResize = false;
		engine->windowWidth = engine->renderer.windowWidth;
		engine->windowHeight = engine->renderer.windowHeight;
	}

	// NOTE: Frame time
	// TODO: Use cpu timing instead
	std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
	std::chrono::duration<f32> frameTime = frameEnd - engine->frameStart;
	state->renderPacket.dt = frameTime.count();
	state->time += state->renderPacket.dt;
	engine->frameStart = frameEnd;

	EngineProcessInput(engine);

	memory->cameraData->view = LookAt(state->player.pos, state->player.pos + state->player.dir, VEC3_UP);
	memory->cameraData->proj = Perspective(state->player.fov, engine->windowWidth / (f32)engine->windowHeight, 0.01f, 100.0f);

	state->renderPacket.playerPos = state->player.pos;

	engine->renderer.DrawFrame(&state->renderPacket);

	// NOTE: Update stuff from vulkan
	memory->cameraData = (camera_data *)state->renderPacket.newCameraBuffer;
	memory->sceneData = (scene_data *)state->renderPacket.newSceneBuffer;
}

extern "C" FUNC_ENGINE_DESTROY(EngineDestroy)
{
	RendererDestroy(&engine->renderer);
}