/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"
#include <bx/rng.h>

namespace
{


#define SAMPLER_POINT_CLAMP  (BGFX_SAMPLER_POINT|BGFX_SAMPLER_UVW_CLAMP)

static float s_texelHalf = 0.0f;

struct PosColorTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_rgba;
	float m_u;
	float m_v;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorTexCoord0Vertex::ms_decl;

void screenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f)
{
	if (3 == bgfx::getAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_decl) )
	{
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_decl);
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

		const float zz = 0.0f;

		const float minx = -_width;
		const float maxx =  _width;
		const float miny = 0.0f;
		const float maxy = _height*2.0f;

		const float texelHalfW = s_texelHalf/_textureWidth;
		const float texelHalfH = s_texelHalf/_textureHeight;
		const float minu = -1.0f + texelHalfW;
		const float maxu =  1.0f + texelHalfW;

		float minv = texelHalfH;
		float maxv = 2.0f + texelHalfH;

		if (_originBottomLeft)
		{
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.0f;
			maxv -= 1.0f;
		}

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_rgba = 0xffffffff;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_rgba = 0xffffffff;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_rgba = 0xffffffff;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		bgfx::setVertexBuffer(0, &vb);
	}
}

class ExampleHDR : public entry::AppI
{
public:
	ExampleHDR(const char* _name, const char* _description)
		: entry::AppI(_name, _description)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable m_debug text.
		bgfx::setDebug(m_debug);

		const bgfx::Caps* caps = bgfx::getCaps();
		m_computeSupported = !!(caps->supported & BGFX_CAPS_COMPUTE);

		if (!m_computeSupported) {
			return;
		}

		// Create vertex stream declaration.
		PosColorTexCoord0Vertex::init();

		m_envTexture = loadTexture("textures/papermill.ktx"
				, 0
				| BGFX_SAMPLER_U_CLAMP
				| BGFX_SAMPLER_V_CLAMP
				| BGFX_SAMPLER_W_CLAMP
				);

		s_texCube   = bgfx::createUniform("s_texCube",  bgfx::UniformType::Sampler);
		s_texColor  = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		s_texAvgLum = bgfx::createUniform("s_texAvgLum", bgfx::UniformType::Sampler);
		u_mtx       = bgfx::createUniform("u_mtx",      bgfx::UniformType::Mat4);
		u_tonemap = bgfx::createUniform("u_tonemap", bgfx::UniformType::Vec4);
		u_histogramParams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);

		m_skyProgram     = loadProgram("vs_tonemapping_skybox",  "fs_tonemapping_skybox");
		m_meshProgram    = loadProgram("vs_tonemapping_mesh", "fs_tonemapping_mesh");
		m_histogramProgram = loadProgram("cs_lum_hist", NULL);
		m_averagingProgram = loadProgram("cs_lum_avg", NULL);
		m_tonemapProgram = loadProgram("vs_tonemapping_tonemap", "fs_tonemapping_tonemap");
		m_mesh = meshLoad("meshes/bunny.bin");

		m_fbh.idx = bgfx::kInvalidHandle;

		m_histogramBuffer = bgfx::createDynamicIndexBuffer(256, BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
		
		// Imgui.
		imguiCreate();

		m_caps = bgfx::getCaps();
		s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

		m_oldWidth  = 0;
		m_oldHeight = 0;
		m_oldReset  = m_reset;

		m_speed      = 0.37f;
		m_middleGray = 0.18f;
		m_white      = 3.0f;
		m_threshold  = 1.5f;

		m_scrollArea = 0;

		m_time = 0.0f;
	}

	virtual int shutdown() override
	{
		// Cleanup.
		imguiDestroy();

		if (!m_computeSupported) {
			return 0;
		}

		meshUnload(m_mesh);

		if (bgfx::isValid(m_fbh) )
		{
			bgfx::destroy(m_fbh);
		}

		bgfx::destroy(m_meshProgram);
		bgfx::destroy(m_skyProgram);
		bgfx::destroy(m_tonemapProgram);
		bgfx::destroy(m_histogramProgram);
		bgfx::destroy(m_averagingProgram);

		bgfx::destroy(m_envTexture);

		bgfx::destroy(m_histogramBuffer);
		bgfx::destroy(m_lumAvgTarget);

		bgfx::destroy(s_texCube);
		bgfx::destroy(s_texColor);
		bgfx::destroy(s_texAvgLum);
		bgfx::destroy(u_mtx);
		bgfx::destroy(u_tonemap);
		bgfx::destroy(u_histogramParams);

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{

			if (!m_computeSupported) {
				return false;
			}

			if (!bgfx::isValid(m_fbh)
			||  m_oldWidth  != m_width
			||  m_oldHeight != m_height
			||  m_oldReset  != m_reset)
			{
				// Recreate variable size render targets when resolution changes.
				m_oldWidth  = m_width;
				m_oldHeight = m_height;
				m_oldReset  = m_reset;

				uint32_t msaa = (m_reset&BGFX_RESET_MSAA_MASK)>>BGFX_RESET_MSAA_SHIFT;

				if (bgfx::isValid(m_fbh) )
				{
					bgfx::destroy(m_fbh);
				}

				m_fbtextures[0] = bgfx::createTexture2D(
					  uint16_t(m_width)
					, uint16_t(m_height)
					, false
					, 1
					, bgfx::TextureFormat::RGBA16F
					, (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
					);

				const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY|(uint64_t(msaa+1)<<BGFX_TEXTURE_RT_MSAA_SHIFT);

				bgfx::TextureFormat::Enum depthFormat =
					  bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16,   textureFlags) ? bgfx::TextureFormat::D16
					: bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags) ? bgfx::TextureFormat::D24S8
					: bgfx::TextureFormat::D32
					;

				m_fbtextures[1] = bgfx::createTexture2D(
					  uint16_t(m_width)
					, uint16_t(m_height)
					, false
					, 1
					, depthFormat
					, textureFlags
					);

				m_fbh = bgfx::createFrameBuffer(BX_COUNTOF(m_fbtextures), m_fbtextures, true);

				uint64_t lumAvgFlags = BGFX_TEXTURE_COMPUTE_WRITE | SAMPLER_POINT_CLAMP;
				m_lumAvgTarget = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::R16F, lumAvgFlags);
				bgfx::setName(m_lumAvgTarget, "LumAvgTarget");
			}

			imguiBeginFrame(m_mouseState.m_mx
					,  m_mouseState.m_my
					, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
					,  m_mouseState.m_mz
					, uint16_t(m_width)
					, uint16_t(m_height)
					);

			showExampleDialog(this);

			ImGui::SetNextWindowPos(
				  ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::SetNextWindowSize(
				  ImVec2(m_width / 5.0f, m_height / 2.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::Begin("Settings"
				, NULL
				, 0
				);

			ImGui::SliderFloat("Speed", &m_speed, 0.0f, 1.0f);
			ImGui::Separator();

			ImGui::SliderFloat("Middle gray", &m_middleGray, 0.1f, 1.0f);
			ImGui::SliderFloat("White point", &m_white,      0.1f, 5.0f);
			ImGui::SliderFloat("Threshold",   &m_threshold,  0.1f, 2.0f);

			ImGui::End();

			imguiEndFrame();

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			int64_t now = bx::getHPCounter();
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency() );

			m_time += (float)(frameTime*m_speed/freq);

			bgfx::ViewId hdrSkybox       = 0;
			bgfx::ViewId hdrMesh         = 1;
			bgfx::ViewId histogramPass = 2;
			bgfx::ViewId averagingPass = 3;
			bgfx::ViewId hdrHBlurTonemap = 4;

			// Set views.
			bgfx::setViewName(hdrSkybox, "Skybox");
			bgfx::setViewClear(hdrSkybox, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
			bgfx::setViewRect(hdrSkybox, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewFrameBuffer(hdrSkybox, m_fbh);

			bgfx::setViewName(hdrMesh, "Mesh");
			bgfx::setViewClear(hdrMesh, BGFX_CLEAR_DISCARD_DEPTH | BGFX_CLEAR_DISCARD_STENCIL);
			bgfx::setViewRect(hdrMesh, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewFrameBuffer(hdrMesh, m_fbh);

			bgfx::setViewName(histogramPass, "Luminence Histogram");

			bgfx::setViewName(averagingPass, "Avergaing the Luminence Histogram");

			bgfx::setViewName(hdrHBlurTonemap, "Tonemap");
			bgfx::setViewRect(hdrHBlurTonemap, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::FrameBufferHandle invalid = BGFX_INVALID_HANDLE;
			bgfx::setViewFrameBuffer(hdrHBlurTonemap, invalid);

			const bgfx::Caps* caps = bgfx::getCaps();
			float proj[16];
			bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth);

			for (uint8_t ii = 0; ii <= hdrHBlurTonemap; ++ii)
			{
				bgfx::setViewTransform(ii, NULL, proj);
			}

			const bx::Vec3 at  = { 0.0f, 1.0f,  0.0f };
			const bx::Vec3 eye = { 0.0f, 1.0f, -2.5f };

			float mtx[16];
			bx::mtxRotateXY(mtx
					, 0.0f
					, m_time
					);

			const bx::Vec3 tmp = bx::mul(eye, mtx);

			float view[16];
			bx::mtxLookAt(view, tmp, at);
			bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, caps->homogeneousDepth);

			// Set view and projection matrix for view hdrMesh.
			bgfx::setViewTransform(hdrMesh, view, proj);

			// Render skybox into view hdrSkybox.
			bgfx::setTexture(0, s_texCube, m_envTexture);
			bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A);
			bgfx::setUniform(u_mtx, mtx);
			screenSpaceQuad( (float)m_width, (float)m_height, true);
			bgfx::submit(hdrSkybox, m_skyProgram);

			// Render m_mesh into view hdrMesh.
			bgfx::setTexture(0, s_texCube, m_envTexture);
			meshSubmit(m_mesh, hdrMesh, m_meshProgram, NULL);


			float minLogLum = -8.0f;
			float maxLogLum = 3.5f;
			float params[4] = {
				minLogLum,
				1.0f / (maxLogLum - minLogLum),
				float(m_width),
				float(m_height),
			};
			uint32_t groupsX = static_cast<uint32_t>(bx::ceil(m_width / 16.0f));
			uint32_t groupsY = static_cast<uint32_t>(bx::ceil(m_height / 16.0f));
			bgfx::setImage(0, m_fbtextures[0], 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA16F);
			bgfx::setBuffer(1, m_histogramBuffer, bgfx::Access::Write);
			bgfx::setUniform(u_histogramParams, params);
			bgfx::dispatch(histogramPass, m_histogramProgram, groupsX, groupsY, 1);

			float tau = 1.1f;
			float timeCoeff = bx::clamp<float>(1.0f - bx::exp(-frameTime * tau), 0.0, 1.0);
			float avgParams[4] = {
				minLogLum,
				maxLogLum - minLogLum,
				timeCoeff,
				static_cast<float>(m_width * m_height),
			};
			bgfx::setImage(0, m_lumAvgTarget, 0, bgfx::Access::ReadWrite, bgfx::TextureFormat::R16F);
			bgfx::setBuffer(1, m_histogramBuffer, bgfx::Access::ReadWrite);
			bgfx::setUniform(u_histogramParams, avgParams);
			bgfx::dispatch(averagingPass, m_averagingProgram, 1, 1, 1);

			float tonemap[4] = { m_middleGray, bx::square(m_white), m_threshold, m_time };
			bgfx::setTexture(0, s_texColor, m_fbtextures[0]);
			bgfx::setTexture(1, s_texAvgLum, m_lumAvgTarget, SAMPLER_POINT_CLAMP);
			bgfx::setUniform(u_tonemap, tonemap);
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			screenSpaceQuad((float)m_width, (float)m_height, m_caps->originBottomLeft);
			bgfx::submit(hdrHBlurTonemap, m_tonemapProgram);

			bgfx::frame();

			m_firstFrame = false;

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	bgfx::ProgramHandle m_skyProgram;
	bgfx::ProgramHandle m_meshProgram;
	bgfx::ProgramHandle m_tonemapProgram;
	bgfx::ProgramHandle m_histogramProgram;
	bgfx::ProgramHandle m_averagingProgram;

	bgfx::TextureHandle m_envTexture;
	bgfx::UniformHandle s_texCube;
	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle s_texAvgLum;
	bgfx::UniformHandle u_mtx;
	bgfx::UniformHandle u_tonemap;
	bgfx::UniformHandle u_histogramParams;

	Mesh* m_mesh;

	bgfx::DynamicIndexBufferHandle m_histogramBuffer;

	bgfx::TextureHandle m_fbtextures[2];
	bgfx::TextureHandle m_lumAvgTarget;
	bgfx::FrameBufferHandle m_fbh;

	bx::RngMwc m_rng;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;

	uint32_t m_oldWidth;
	uint32_t m_oldHeight;
	uint32_t m_oldReset;

	float m_speed;
	float m_middleGray;
	float m_white;
	float m_threshold;

	int32_t m_scrollArea;

	const bgfx::Caps* m_caps;
	float m_time;

	bool m_computeSupported;
	bool m_firstFrame = true;

};

} // namespace

ENTRY_IMPLEMENT_MAIN(ExampleHDR, "41-tonemapping", "Using multiple views with frame buffers, and view order remapping.");
