#ifndef __ITEMDEFINATION_H__
#define __ITEMDEFINATION_H__

#include <string>
#include <stdint.h>

using std::string;

enum ItemType
{
	ItemTypeStaticMapItem,
	ItemTypeTimedMapItem,
	ItemTypeAnimatedMapItem
};

class MapItemDefination {
private:
	string m_dffStem;
	string m_txdStem;
	float m_drawDistances;
	unsigned int m_flags;
	bool m_lod;
	
public:
	MapItemDefination();
	void SetDFFStem(string dffStem);
	void SetTexDictStem(string txdStem);
	void SetFlags(uint32_t n);
	void SetDrawDistance(float drawdist) ;
	void SetLodProperty(bool bLod);
	
	string GetDFFStem();
	string GetTexDictStem();
	float GetDrawDistance();
	bool IsLod();
	bool hasAlphaTransparency();
	
	virtual ItemType getType() const = 0;
	
};

class StaticMapItemDefination: public MapItemDefination
{
public:
	virtual ItemType getType() const {return ItemType::ItemTypeStaticMapItem;}
};

#endif