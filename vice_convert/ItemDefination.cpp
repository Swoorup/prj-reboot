#include "ItemDefination.h"

MapItemDefination::MapItemDefination()
{
	m_dffStem = m_txdStem = "";
	m_flags = 0;
	m_lod = false;
	m_drawDistances = 0.0f;
}


void MapItemDefination::SetDFFStem(string dffStem) {
	m_dffStem = dffStem;
}

void MapItemDefination::SetTexDictStem(string txdStem) {
	m_txdStem = txdStem;
}

void MapItemDefination::SetFlags(uint32_t n) {
	m_flags = n;
}

void MapItemDefination::SetDrawDistance(float drawdist) {
	m_drawDistances = drawdist;
}

string MapItemDefination::GetDFFStem() {
	return m_dffStem;
}

string MapItemDefination::GetTexDictStem() {
	return m_txdStem;
}

float MapItemDefination::GetDrawDistance() {
	return m_drawDistances;
}

bool MapItemDefination::hasAlphaTransparency() {
	return ((m_flags & (1 << 2)) != 0);
}

void MapItemDefination::SetLodProperty(bool bLod)
{
	m_lod = bLod;
}

bool MapItemDefination::IsLod()
{
	return m_lod;
}