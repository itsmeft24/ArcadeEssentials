#pragma once

namespace Flash {
	class EngineTextureSet;
	class Layer {
	public:
		bool m_isUpdating;
		bool m_unk;
		char* m_name;
	public:
		virtual ~Layer() = 0;
	};
};