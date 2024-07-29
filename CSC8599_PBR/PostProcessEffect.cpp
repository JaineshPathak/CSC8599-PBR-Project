#include "PostProcessEffect.h"

PostProcessEffect::PostProcessEffect(const unsigned int& sizeX, const unsigned int& sizeY) :
	m_WidthF((float)sizeX), m_HeightF((float)sizeY),
	m_WidthI(sizeX), m_HeightI(sizeY), m_PostEffectType(EPostEffectType::Type_None)
{
	m_QuadMesh = Renderer::Get()->GetQuadMesh();
	m_SrcViewportSize = Vector2(m_WidthF, m_HeightF);
}