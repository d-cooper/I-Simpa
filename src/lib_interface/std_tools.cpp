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

#include "std_tools.hpp"
#include <math.h>
#include <float.h>
#include <boost/filesystem.hpp>

#ifdef _WIN32
	#include <direct.h>
	#include "input_output/pugixml/src/pugixml.hpp"
#endif
#ifdef _UNIX
	#include <sys/stat.h>
    #include <sys/types.h>
#endif

bool st_mkdir(const std::string& pathname)
{
    #ifdef _WIN32
	// convert path to wide char under win32
		return boost::filesystem::create_directories(pugi::as_wide(pathname));
	#else
		return boost::filesystem::create_directories(pathname);
	#endif
}

std::string st_path_separator() {
    boost::filesystem::path path("");
    path += boost::filesystem::path::preferred_separator;
    return path.string();
}


bool st_isfinite(const float& value)
{
	#ifdef _MSC_VER
		return _finite(value);
	#else
		return std::isfinite(value);
	#endif
}
