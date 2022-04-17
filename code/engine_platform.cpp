#include "engine_platform.h"

//------------------------------------------------------------------------
// GLOBALS

global_var platform_api *platformAPI; // This is also used in vulkan_platform.

//------------------------------------------------------------------------

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
	platformAPI = &engine->platformAPI;

	engine->input = {};
	engine->onResize = false;

	engine_memory *memory = &engine->memory;
	engine_state *state = engine->state = (engine_state *)memory->permanentMemory;

	MemoryArenaInitialize(&state->memoryArena, (u8 *)memory->permanentMemory + sizeof(engine_state), memory->permanentMemorySize - sizeof(engine_state));
	memory->isInitialized = true;

	// NOTE: Everything is initialized here
	state->renderPacket.clearColor = { 0.0f, 0.4f, 0.3f };

	if (!RendererInitialize(RENDERER_GRAPHICS_API_VULKAN, engine))
	{
		DebugPrintFunctionResult(false);
		Assert(0);
	}

	platformAPI->GetFileWriteTime(DEFAULT_SHADER_FILEPATH, &engine->shadersWriteTime);

	shader_resource_bind shaderBinds[] = {
		{ SHADER_STAGE_FLAG_VERTEX, SHADER_RESOURCE_TYPE_UNIFORM, 0, sizeof(camera_ubo), true, (void **)&memory->cameraUBO },
		{ SHADER_STAGE_FLAG_FRAGMENT, SHADER_RESOURCE_TYPE_UNIFORM, 1, sizeof(scene_ubo), true, (void **)&memory->sceneUBO },
		{ SHADER_STAGE_FLAG_VERTEX, SHADER_RESOURCE_TYPE_STORAGE, 2, sizeof(object_transform) * MAX_OBJECT_TRANSFORMS, true, (void **)&memory->transforms },
	};
	VulkanBindShaderResources(shaderBinds, ArrayCount(shaderBinds));

	memory->stageBuffer.memory = VulkanRequestBuffer(RENDERER_BUFFER_TYPE_STAGING, MEGABYTES(256), false);
	memory->stageBuffer.nextWrite = memory->stageBuffer.memory;

	VulkanCreateGrid(); // TODO: This uses the stage buffer. But it should be done in the engine.

	scene_resources sceneResources = {};
	CreateScene(memory->stageBuffer.nextWrite, sceneResources, state->renderPacket.scene, *memory->sceneUBO);

	VulkanUpload(sceneResources);

	CameraInitialize(state->player, { 0.0f, 2.0f, -5.0f }, { 0.0f, 0.0f, -1.0f }, VEC3_UP, 4.0f, 2.0f, 60.0f);

	engine->input.mouse.cursorEnabled = true;
	engine->input.mouse.firstMove = true;

	state->time = 0.0f;
	engine->frameStart = std::chrono::steady_clock::now();
}

inline static_func void EngineProcessInput(engine_platform *engine)
{
	engine_input &input = engine->input;
	engine_state *state = engine->state;
	renderer_platform &renderer = engine->renderer;

	// NOTE(heyyod): SETTINGS CONTROL
	if (!input.keyboard.isPressed[KEY_ALT] && !input.keyboard.altWasUp)
		input.keyboard.altWasUp = true;
	if (input.keyboard.isPressed[KEY_ALT] && input.keyboard.altWasUp)
	{

		CHANGE_GRAPHICS_SETTINGS newSettings = CHANGE_NONE;
		MSAA_OPTIONS newMSAA = state->settings.msaa;

		if (input.keyboard.isPressed[KEY_ONE])
			newMSAA = MSAA_OFF;
		if (input.keyboard.isPressed[KEY_TWO])
			newMSAA = MSAA_2;
		if (input.keyboard.isPressed[KEY_THREE])
			newMSAA = MSAA_4;
		if (input.keyboard.isPressed[KEY_FOUR])
			newMSAA = MSAA_8;
		if (newMSAA != state->settings.msaa)
		{
			state->settings.msaa = newMSAA;
			newSettings |= CHANGE_MSAA;
			input.keyboard.altWasUp = false;
		}
		if (newSettings)
			renderer.ChangeGraphicsSettings(state->settings, newSettings);

		if (input.keyboard.isPressed[KEY_TILDE])
		{
			input.mouse.cursorEnabled = !input.mouse.cursorEnabled;
			input.mouse.firstMove = true;
			input.keyboard.altWasUp = false;
		}
	}

	// NOTE(heyyod): CAMERA CONTROL
	input.mouse.cursorEnabled = !input.mouse.rightIsPressed;
	if (input.mouse.rightIsPressed)
	{
		input.mouse.cursorEnabled = false;

		camera &player = state->player;
		vec3 dPos = {};
		vec2 dLook = {};

		if (input.keyboard.isPressed[KEY_W])
			dPos += player.dir;
		if (input.keyboard.isPressed[KEY_S])
			dPos -= player.dir;
		if (input.keyboard.isPressed[KEY_A])
			dPos += Cross(VEC3_UP, player.dir);
		if (input.keyboard.isPressed[KEY_D])
			dPos += Cross(player.dir, VEC3_UP);
		if (input.keyboard.isPressed[KEY_Q])
			dPos.Y -= 1.0f;
		if (input.keyboard.isPressed[KEY_E])
			dPos.Y += 1.0f;
		dPos *= state->renderPacket.dt;

		if (input.keyboard.isPressed[KEY_SHIFT])
			dPos *= 6.0f; // SPEED BOOST

		if (input.mouse.firstMove)
		{
			input.mouse.lastPos = input.mouse.newPos;
			input.mouse.firstMove = false;
		}

		dLook = {
			input.mouse.newPos.X - input.mouse.lastPos.X,
			input.mouse.lastPos.Y - input.mouse.newPos.Y
		};
		dLook *= state->renderPacket.dt;
		CameraUpdate(player, dPos, dLook);
	}
	input.mouse.lastPos = input.mouse.newPos;
}

extern "C" FUNC_ENGINE_UPDATE_AND_RENDER(EngineUpdateAndRender)
{
	Assert(engine->memory.isInitialized);

	engine_memory *memory = &engine->memory;
	engine_state *state = engine->state = (engine_state *)memory->permanentMemory;
	renderer_platform &renderer = engine->renderer;

	if (!renderer.canRender)
		return;

	if (engine->reloaded)
	{
		RendererOnEngineReload(engine);
		platformAPI = &engine->platformAPI;
		engine->reloaded = false;
	}

	// Check if shader have been updated
	if (platformAPI->WasFileUpdated(DEFAULT_SHADER_FILEPATH, &engine->shadersWriteTime))
	{
		renderer.OnShaderReload();
	}

	if (engine->onResize)
	{
		renderer.OnResize(&renderer);

		engine->onResize = false;
		engine->windowWidth = renderer.windowWidth;
		engine->windowHeight = renderer.windowHeight;
	}

	// NOTE: Frame time
	// TODO: Use cpu timing instead
	std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
	std::chrono::duration<f32> frameTime = frameEnd - engine->frameStart;
	state->renderPacket.dt = frameTime.count();
	state->time += state->renderPacket.dt;
	engine->frameStart = frameEnd;

	EngineProcessInput(engine);

	memory->cameraUBO->view = LookAt(state->player.pos, state->player.pos + state->player.dir, VEC3_UP);
	memory->cameraUBO->proj = Perspective(state->player.fov, engine->windowWidth / (f32)engine->windowHeight, 0.01f, 100.0f);

	state->renderPacket.playerPos = state->player.pos;

	//------------------------------------------------------------------------
	// TEMP
	state->renderPacket.scene.loadedMeshes[0].count = 4U;

	f32 scaleFactor = Abs(CosF(state->time)) + 0.5f;
	object_transform *t = (object_transform *)memory->transforms;

	t[0].model = Translate( { SinF(state->time), 0.0f, 0.0f }) *
		Rotate(-90.0f, { 0.0f, 1.0f, 0.0f }) *
		Rotate(-90.0f, { 1.0f, 0.0f, 0.0f }) *
		Scale( { scaleFactor, scaleFactor, scaleFactor });

	t[1].model = Translate( { 3.0, SinF(state->time), CosF(state->time) }) *
		Rotate(-90.0f, { 0.0f, 1.0f, 0.0f }) *
		Rotate(-90.0f, { 1.0f, 0.0f, 0.0f });

	t[2].model = Translate( { 6.0f, 0.0f, 0.0f }) *
		Rotate(-90.0f, { 0.0f, 1.0f, 0.0f }) *
		Rotate(-90.0f, { 1.0f, 0.0f, 0.0f }) *
		Scale( { scaleFactor, scaleFactor, scaleFactor });

	t[3].model = Translate( { -6.0f, SinF(state->time), CosF(state->time) }) *
		Rotate(-90.0f, { 0.0f, 1.0f, 0.0f }) *
		Rotate(-90.0f, { 1.0f, 0.0f, 0.0f });
	//------------------------------------------------------------------------

	renderer.DrawFrame(&state->renderPacket);

	memory->cameraUBO = (camera_ubo *)state->renderPacket.nextCameraPtr;
	memory->sceneUBO = (scene_ubo *)state->renderPacket.nextScenePtr;
	memory->transforms = (object_transform *)state->renderPacket.nextTransformsPtr;
}

extern "C" FUNC_ENGINE_DESTROY(EngineDestroy)
{
	RendererDestroy(&engine->renderer);
}
