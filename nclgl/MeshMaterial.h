#pragma once
#include <string>
#include <map>
#include <vector>
#include <unordered_set>

using std::map;
using std::string;
using std::vector;

class MeshMaterialEntry {
public:
	std::map<string, string> entries;

	bool GetEntry(const string& name, const string** output) const {
		auto i = entries.find(name);
		if (i == entries.end()) {
			return false;
		}
		*output = &i->second;
		return true;
	}

};

class MeshMaterial
{
public:
	MeshMaterial(const std::string& filename);
	MeshMaterial(const std::string& filename, bool ignoreMeshDir);
	~MeshMaterial() {}
	const MeshMaterialEntry* GetMaterialForLayer(int i) const;

	const std::unordered_set<std::string>& GetTexturesList() const { return texturesList; }
protected:
	std::vector<MeshMaterialEntry>	materialLayers;
	std::vector<MeshMaterialEntry*> meshLayers;
	std::unordered_set<std::string> texturesList;
};

