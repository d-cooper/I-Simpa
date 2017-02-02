/* ----------------------------------------------------------------------
* I-SIMPA (http://i-simpa.ifsttar.fr). This file is part of I-SIMPA.
*
* I-SIMPA is a GUI for 3D numerical sound propagation modelling dedicated
* to scientific acoustic simulations.
* Copyright (C) 2007-2014 - IFSTTAR - Judicael Picaut, Nicolas Fortin
*
* I-SIMPA is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* 
* I-SIMPA is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA or 
* see <http://ww.gnu.org/licenses/>
*
* For more information, please consult: <http://i-simpa.ifsttar.fr> or 
* send an email to i-simpa@ifsttar.fr
*
* To contact Ifsttar, write to Ifsttar, 14-20 Boulevard Newton
* Cite Descartes, Champs sur Marne F-77447 Marne la Vallee Cedex 2 FRANCE
* or write to scientific.computing@ifsttar.fr
* ----------------------------------------------------------------------*/

#include "surf_merging.hpp"
#include <tools/octree44_dotElement.hpp>
#include "../Core/mathlib.h"


namespace surf_merge
{
#define VERT_TEST(i,v3) (i==v3.a || i==v3.b || i==v3.c)

	struct group_t
	{
		group_t()
			:matid(-1)
		{

		}
		int matid;
		std::list<std::size_t> faceIds;
	};
	struct facemodel_t
	{
		facemodel_t():idmat(0),processed(false){}
		bool processed;
		std::list<std::size_t> neigh[3];
		vec3 normal;
		unsigned int idmat;
	};
	struct SurfaceMerging::SelfData_t
	{
		std::list<group_t> groups;
		std::vector<facemodel_t> faces; //faces du mod�le
	};
	bool IsSameVertices( const ivec3& vertsFace,const ivec2& vertsTest)
	{
		if(VERT_TEST(vertsTest.a,vertsFace) && VERT_TEST(vertsTest.b,vertsFace))
			return true;
		else
			return false;
	}

	SurfaceMerging::SurfaceMerging()
		:self(new SelfData_t())
	{
	}
	SurfaceMerging::~SurfaceMerging()
	{
		delete self;
	}
	void FillNeighboor( const std::size_t& currentFaceIndex, std::list<std::size_t>& possibleNeighboorAtVertOne,const ivec2& segment, const int& side,const formatCoreBIN::ioModel& model,std::vector<int>& modelv_to_merged_v,std::vector<facemodel_t>& faces)
	{
		const formatCoreBIN::ioFace& curface=model.faces[currentFaceIndex];
		facemodel_t& curlocateface=faces[currentFaceIndex];
		for(std::list<std::size_t>::const_iterator itfrom=possibleNeighboorAtVertOne.begin();itfrom!=possibleNeighboorAtVertOne.end();itfrom++)
		{
			if(*itfrom!=currentFaceIndex && IsSameVertices(ivec3(modelv_to_merged_v[model.faces[*itfrom].a],modelv_to_merged_v[model.faces[*itfrom].b],modelv_to_merged_v[model.faces[*itfrom].c]),segment))
			{
				curlocateface.neigh[side].push_back(*itfrom);
			}
		}
	}

	int SurfaceMerging::GetGroups()
	{
		return self->groups.size();
	}
	void FillGroup(group_t& groupToFill,facemodel_t& faceToPush,const std::size_t& faceIndexToPush,std::vector<facemodel_t>& faces)
	{
		groupToFill.faceIds.push_back(faceIndexToPush);
		faceToPush.processed=true;
		for(unsigned short idside=0;idside<3;idside++)
		{
			if(faceToPush.neigh[idside].size()==1) //si plus de 1 voisin sur un cot� on limite le cot�
			{
				std::size_t idneigh=faceToPush.neigh[idside].front();
				facemodel_t& neighboor=faces[idneigh];
				if(!neighboor.processed && neighboor.idmat==faceToPush.idmat && (neighboor.normal.barelyEqual(faceToPush.normal) || neighboor.normal.barelyEqual(faceToPush.normal*-1) ))
					FillGroup(groupToFill,neighboor,idneigh,faces);
			}
			/*
			for(std::list<std::size_t>::iterator it=faceToPush.neigh.begin();it!=faceToPush.neigh.end();it++)
			{
				facemodel_t& neighboor=faces[*it];
				if(!neighboor.processed && neighboor.idmat==faceToPush.idmat && neighboor.normal.barelyEqual(faceToPush.normal))
					FillGroup(groupToFill,neighboor,*it);
			}
			*/
		}
	}
	void SurfaceMerging::LoadGroups(const formatCoreBIN::ioModel& model)
	{
		self->groups.clear();
		self->faces.clear();
		self->faces.insert(self->faces.end(),model.faces.size(),facemodel_t());
		//////////////////////////////////////////////
		//Fusion des sommets ayant les m�mes positions
		std::vector<int> modelv_to_merged_v(model.vertices.size(),-1);
		std::vector<vec3> merged_vert;
		merged_vert.reserve(model.vertices.size());
		using namespace octreeTool;
		spaceElementContainer vertsSpaceEl;
		vertsSpaceEl.reserve(model.vertices.size());
		for(std::vector<formatCoreBIN::t_pos>::const_iterator itvert=model.vertices.begin();itvert<model.vertices.end();itvert++)
			vertsSpaceEl.push_back(new dotElement(vec3((*itvert).v)));
		Octree44 modelOctree(vertsSpaceEl,5);
		for(std::vector<spaceElement*>::iterator itdel=vertsSpaceEl.begin();itdel!=vertsSpaceEl.end();itdel++)
			delete (*itdel);
		vertsSpaceEl.clear();
		std::size_t idvert=0;
		std::size_t idmergedvert=0;
		for(std::vector<formatCoreBIN::t_pos>::const_iterator itvert=model.vertices.begin();itvert<model.vertices.end();itvert++)
		{
			if(modelv_to_merged_v[idvert]==-1)
			{
				vec3 posv=vec3((*itvert).v);
				idmergedvert=merged_vert.size();
				merged_vert.push_back(posv);

				dotElement dottest(posv);
				std::vector<octreeTool::elementSize> sameDotCandidates;
				modelOctree.GetAllCollisionCandidates(dottest,sameDotCandidates);
				for(std::vector<octreeTool::elementSize>::iterator itcandidates=sameDotCandidates.begin();itcandidates<sameDotCandidates.end();itcandidates++)
				{
					if(posv.barelyEqual(vec3(model.vertices[*itcandidates].v)))
						modelv_to_merged_v[*itcandidates]=idmergedvert;
				}
			}
			idvert++;
		}
		//Initialisation des faces locales
		std::size_t faceId=0;
		for(std::vector<facemodel_t>::iterator it=self->faces.begin();it!=self->faces.end();it++)
		{
			const formatCoreBIN::ioFace& currentFace=model.faces[faceId];
			(*it).idmat=currentFace.idMat;
			(*it).normal=FaceNormal<float>((const float*)&model.vertices[currentFace.a],(const float*)&model.vertices[currentFace.b],(const float*)&model.vertices[currentFace.c]);
			faceId++;
		}
		//Initialisation des faces voisines
		typedef std::list<std::size_t> t_listofint;
		std::vector<t_listofint> dotToFace(merged_vert.size());

		std::size_t curface=0;
		//Construction du tableau de noeud>face
		for(std::vector<formatCoreBIN::ioFace>::const_iterator it=model.faces.begin();it!=model.faces.end();it++)
		{
			const formatCoreBIN::ioFace& currentFace=*(it);
			dotToFace[modelv_to_merged_v[currentFace.a]].push_back(curface);
			dotToFace[modelv_to_merged_v[currentFace.b]].push_back(curface);
			dotToFace[modelv_to_merged_v[currentFace.c]].push_back(curface);
			curface++;
		}
		//Construction des voisins de chaque face
		curface=0;
		for(std::vector<formatCoreBIN::ioFace>::const_iterator it=model.faces.begin();it!=model.faces.end();it++)
		{
			const formatCoreBIN::ioFace& currentFace=*(it);
			FillNeighboor( curface, dotToFace[modelv_to_merged_v[currentFace.a]],ivec2(modelv_to_merged_v[currentFace.a],modelv_to_merged_v[currentFace.b]), 0,model,modelv_to_merged_v,self->faces);
			FillNeighboor( curface, dotToFace[modelv_to_merged_v[currentFace.b]],ivec2(modelv_to_merged_v[currentFace.b],modelv_to_merged_v[currentFace.c]), 1,model,modelv_to_merged_v,self->faces);
			FillNeighboor( curface, dotToFace[modelv_to_merged_v[currentFace.c]],ivec2(modelv_to_merged_v[currentFace.c],modelv_to_merged_v[currentFace.a]), 2,model,modelv_to_merged_v,self->faces);
			curface++;
		}

		//Construction des groupes de surfaces
		facemodel_t* faceToProcess=&self->faces[0];
		std::size_t nextFaceToProcess=1;
		while(faceToProcess!=NULL)
		{
			self->groups.push_back(group_t());
			group_t& newGroup=self->groups.back();
			newGroup.matid=faceToProcess->idmat;
			FillGroup(newGroup,*faceToProcess,nextFaceToProcess-1,self->faces);
			faceToProcess=NULL;
			while(faceToProcess==NULL && nextFaceToProcess<self->faces.size())
			{
				if(!self->faces[nextFaceToProcess].processed)
					faceToProcess=&self->faces[nextFaceToProcess];
				nextFaceToProcess++;
			}
		}
	}

	void SurfaceMerging::GetGroupInformation(const unsigned int& idel,std::vector<std::size_t>& faceIds)
	{
		std::size_t counter=0;
		for(std::list<group_t>::iterator it=self->groups.begin();it!=self->groups.end();it++)
		{
			if(counter==idel)
			{
				faceIds.reserve((*it).faceIds.size());
				for(std::list<std::size_t>::iterator itface=(*it).faceIds.begin();itface!=(*it).faceIds.end();itface++)
					faceIds.push_back(*itface);
				break;
			}
			counter++;
		}
	}
	void SurfaceMerging::SavePolyToPolyFile(const char *strFileName)
	{
		//1ere �tape
		//Il faut diviser les groupes en polygones


	}
};
