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

#include "e_directivity.h"
#include "data_manager/e_data_file.h"
#include "data_manager/appconfig.h"
#include <wx/filename.h>
#include "last_cpp_include.hpp"

E_Directivity::E_Directivity(Element* parent, wxString Nom, ELEMENT_TYPE _type, wxXmlNode* nodeElement)
	:Element(parent, Nom, _type, nodeElement)
{
	SetIcon(GRAPH_STATE_ALL, GRAPH_DIRECTIVITY);
	idDirectivity = -1;
	wxString propVal;

	if (nodeElement != NULL) // && nodeElement->GetAttribute("wxid",&propVal)
	{
		//Element initialis� AVEC Xml
		long lval;
		if (nodeElement->GetAttribute("iddirectivity", &propVal))
		{
			propVal.ToLong(&lval);
			if (ApplicationConfiguration::IsIdDirectivityExist(lval))
			{
				idDirectivity = ApplicationConfiguration::GetFreeDirectivityId();
			} else {
				idDirectivity = lval;
			}
		}

		wxString propValue;

		wxXmlNode* currentChild;
		currentChild = nodeElement->GetChildren();
		// app or user folder
		wxFileName storageFolder("");
		if (_type == ELEMENT_TYPE_DIRECTIVITIES_APP)
		{
			storageFolder.Assign(ApplicationConfiguration::CONST_RESOURCE_DIRECTIVITY_FOLDER);
		}
		else {
			storageFolder.Assign(ApplicationConfiguration::GLOBAL_VAR.cacheFolderPath + "loudspeaker" + wxFileName::GetPathSeparator());
		}

		while (currentChild != NULL)
		{
			if (currentChild->GetAttribute("eid", &propValue))
			{
				long typeEle;
				propValue.ToLong(&typeEle);
				if (typeEle == Element::ELEMENT_TYPE_FILE)
				{
					if(currentChild->GetAttribute("label","") != "Directivity file") {
						currentChild->DeleteAttribute("label");
						currentChild->AddAttribute("label", wxTRANSLATE("Directivity file"));
					}
					E_Data_File* dirFile = new E_Data_File(currentChild, this, storageFolder.GetPath(), _("Open loudspeaker file"), _("TXT files (*.TXT)|*.TXT"));
					this->AppendFils(dirFile);
				}
			}
			currentChild = currentChild->GetNext();
		}
		// Init new properties
		if(!IsPropertyExist("description")) {
			this->AppendPropertyText("description", wxTRANSLATE("Description"), "");
		}
	}else{
		idDirectivity = ApplicationConfiguration::GetFreeDirectivityId();
		Modified(this);
	}

	if (idDirectivity == -1)
	{
		idDirectivity = ApplicationConfiguration::GetFreeDirectivityId();
	}
	ApplicationConfiguration::AddDirectivity(this);
}

E_Directivity::~E_Directivity()
{
	ApplicationConfiguration::DeleteDirectivity(this->elementInfo.xmlIdElement);
}

void E_Directivity::InitProperties()
{
	wxFileName storageFolder (ApplicationConfiguration::GLOBAL_VAR.cacheFolderPath);
	storageFolder.AppendDir("loudspeaker");
	if (!storageFolder.DirExists())
	{
		storageFolder.Mkdir();
	}
	this->AppendPropertyFile("file", wxTRANSLATE("Directivity file"), storageFolder.GetPath(), _("Open loudspeaker file"), _("TXT files (*.TXT)|*.TXT"));
	this->AppendPropertyText("description", wxTRANSLATE("Description"), "");
}

int E_Directivity::GetIdDirectivity()
{
	return this->idDirectivity;
}

wxFileName E_Directivity::GetAssociatedFile()
{
	return this->GetFileConfig("file");
}

void E_Directivity::InitProp()
{

}

Element::ELEMENT_TYPE E_Directivity::GetTypeDireciticity()
{
	return this->elementInfo.typeElement;
}

wxXmlNode* E_Directivity::SaveXMLCoreDoc(wxXmlNode* NoeudParent)
{
	return Element::SaveXMLCoreDoc(NoeudParent);
}

wxXmlNode* E_Directivity::SaveXMLDoc(wxXmlNode* NoeudParent)
{
	wxXmlNode* thisNode = Element::SaveXMLDoc(NoeudParent);
	thisNode->SetName("directivities"); // Nom de la balise xml ( pas d'espace autorise )
	thisNode->DeleteAttribute("iddirectivity");
	thisNode->AddAttribute("iddirectivity", Convertor::ToString(idDirectivity));
	return thisNode;
}

void E_Directivity::Modified(Element* eModif)
{
	Element* pereEModif = eModif->GetElementParent();
	if (pereEModif != NULL)
	{
		Element::Modified(eModif); //Element de base modifi�
	}
}

