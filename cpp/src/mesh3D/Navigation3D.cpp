#include "Navigation3D.hpp"
#include <algorithm>
#include <iterator>
#include <set>
#include <cassert>

using namespace poly_fem::Navigation3D;
using namespace poly_fem;
using namespace std;


namespace
{
	bool predicate(const uint32_t a, const uint32_t b){ return a == b; }
}

	void poly_fem::Navigation3D::prepare_mesh(Mesh3DStorage &M) {
		M.type = MeshType::Hyb;
		poly_fem::MeshProcessing3D::build_connectivity(M);
	}


	Index poly_fem::Navigation3D::get_index_from_element_face(const Mesh3DStorage &M, int hi, int lf, int lv)
	{
		Index idx;

		if (hi >= M.elements.size()) hi = hi % M.elements.size();
		idx.element = hi;

		if (lf >= M.elements[hi].fs.size()) lf = lf % M.elements[hi].fs.size();
		idx.element_patch = lf;
		idx.face = M.elements[hi].fs[idx.element_patch];

		if (lv >= M.faces[idx.face].vs.size()) lv = lv % M.faces[idx.face].vs.size();
		idx.face_corner = lv;
		if (!M.elements[hi].fs_flag[idx.element_patch]) idx.face_corner = M.faces[idx.face].vs.size() - 1 - idx.face_corner;
		idx.vertex = M.faces[idx.face].vs[idx.face_corner];
		idx.edge = M.faces[idx.face].es[idx.face_corner];

		return idx;
	}
// Navigation in a surface mesh
	Index poly_fem::Navigation3D::switch_vertex(const Mesh3DStorage &M, Index idx) {

		if(idx.vertex == M.edges[idx.edge].vs[0])idx.vertex = M.edges[idx.edge].vs[1];
		else idx.vertex = M.edges[idx.edge].vs[0];
		idx.face_corner = std::find(M.faces[idx.face].vs.begin(), M.faces[idx.face].vs.end(), idx.vertex) - M.faces[idx.face].vs.begin();
		if (!M.elements[idx.element].fs_flag[idx.element_patch]) idx.face_corner = M.faces[idx.face].vs.size() - 1 - idx.face_corner;

		return idx;
	}
	Index poly_fem::Navigation3D::switch_edge(const Mesh3DStorage &M, Index idx) {

		vector<uint32_t> ves = M.vertices[idx.vertex].neighbor_es, fes = M.faces[idx.face].es, sharedes;
		sort(fes.begin(), fes.end()); sort(ves.begin(), ves.end());
		set_intersection(fes.begin(), fes.end(), ves.begin(), ves.end(), back_inserter(sharedes));
	assert(sharedes.size() == 2);//true for sure
	if (sharedes[0] == idx.edge) idx.edge = sharedes[1];else idx.edge = sharedes[0];

	return idx;
}
Index poly_fem::Navigation3D::switch_face(const Mesh3DStorage &M, Index idx) {

	vector<uint32_t> efs = M.edges[idx.edge].neighbor_fs, hfs = M.elements[idx.element].fs, sharedfs;
	sort(hfs.begin(), hfs.end()); sort(efs.begin(), efs.end());
	set_intersection(hfs.begin(), hfs.end(), efs.begin(), efs.end(), back_inserter(sharedfs));
	assert(sharedfs.size() == 2);//true for sure
	if (sharedfs[0] == idx.face) idx.face = sharedfs[1]; else idx.face = sharedfs[0];
	idx.face_corner = std::find(M.faces[idx.face].vs.begin(), M.faces[idx.face].vs.end(), idx.vertex) - M.faces[idx.face].vs.begin();
	if (!M.elements[idx.element].fs_flag[idx.element_patch]) idx.face_corner = M.faces[idx.face].vs.size() - 1 - idx.face_corner;

	return idx;
}
Index poly_fem::Navigation3D::switch_element(const Mesh3DStorage &M, Index idx) {

	if (M.faces[idx.face].neighbor_hs.size() == 1) {
		idx.element = -1;
		return idx;
	}
	else {
		if (M.faces[idx.face].neighbor_hs[0] == idx.element)
			idx.element = M.faces[idx.face].neighbor_hs[1];
		else idx.element = M.faces[idx.face].neighbor_hs[0];
	}
	idx.face_corner = std::find(M.faces[idx.face].vs.begin(), M.faces[idx.face].vs.end(), idx.vertex) - M.faces[idx.face].vs.begin();
	if (!M.elements[idx.element].fs_flag[idx.element_patch]) idx.face_corner = M.faces[idx.face].vs.size() - 1 - idx.face_corner;

	return idx;
}