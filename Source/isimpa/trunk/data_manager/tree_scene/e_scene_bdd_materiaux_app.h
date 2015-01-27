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

#include "first_header_include.hpp"

#include "data_manager/tree_scene/e_scene_bdd_materiaux_app_materiau.h"
#include "data_manager/tree_scene/e_scene_bdd_materiaux_app_group.h"
#include "data_manager/appconfig.h"

/** \file e_scene_bdd_materiaux_app.h
   \brief Element représentant un dossier contenant un ou plusieurs matériau et/ou un ou plusieurs groupes de matériaux
*/

/**
   \brief Element représentant un dossier contenant un ou plusieurs matériau et/ou un ou plusieurs groupes de matériaux
*/
class E_Scene_Bdd_Materiaux_Application: public Element
{
public:
  /*!
   *  \callgraph
   */

	E_Scene_Bdd_Materiaux_Application( wxXmlNode* noeudCourant ,  Element* parent)
		:Element(parent,"Reference materials",Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_APP,noeudCourant)
	{
		_("Reference materials");

		SetIcon(GRAPH_STATE_EXPANDED,GRAPH_APPLICATION_MATERIALS_OPEN);
		SetIcon(GRAPH_STATE_NORMAL,GRAPH_APPLICATION_MATERIALS_CLOSE);
		wxXmlNode* noeudCfg=ApplicationConfiguration::GetAppMateriauNode();

		//On initialise les id des éléments
		if(noeudCfg!=NULL)
		{
			wxXmlNode* currentChild;
			currentChild = noeudCfg->GetChildren();
			// On va créer les fils de notre noeudCourant
			wxString propValue;
			while(currentChild!=NULL)
			{
				if(currentChild->GetPropVal("eid",&propValue))
				{
					long typeEle;
					propValue.ToLong(&typeEle);
					if(typeEle==Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_APP_MATERIAU || typeEle==Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_USER_MATERIAU)
						this->AppendFils(new E_Scene_Bdd_Materiaux_App_Materiau(currentChild,this));
					if(typeEle==Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_APP_GROUP || typeEle==Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_USER_GROUP)
						this->AppendFils(new E_Scene_Bdd_Materiaux_App_Group(currentChild,this));
				}
				currentChild = currentChild->GetNext();
			}
		}
	}

	E_Scene_Bdd_Materiaux_Application( Element* parent)
		:Element(parent,"Reference",Element::ELEMENT_TYPE_SCENE_BDD_MATERIAUX_APP)
	{
		SetIcon(GRAPH_STATE_EXPANDED,GRAPH_APPLICATION_MATERIALS_OPEN);
		SetIcon(GRAPH_STATE_NORMAL,GRAPH_APPLICATION_MATERIALS_CLOSE);
	}

	


	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = Element::SaveXMLDocWithoutChildren(NoeudParent);
		thisNode->SetName("appmateriaux"); // Nom de la balise xml ( pas d'espace autorise )

		return thisNode;
	}

	

};
