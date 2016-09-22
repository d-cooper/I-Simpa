# -*- coding: cp1252 -*-
from libsimpa import *
from vec3 import vec3


#Ajout de fonctionnalit� par rapport � la librairie en C++

###########################################################
## Gabe_rw

def ToList(self):
    """
        Retourne les donn�es sous forme de listes python
    """
    lstret=[]
    coltypes=list(self.GetTabTypes())
    func_binding={ GABE_OBJECTTYPE.GABE_OBJECTTYPE_SHORTSTRING : self.ReadColStr,
                   GABE_OBJECTTYPE.GABE_OBJECTTYPE_INT : self.ReadColInt,
                   GABE_OBJECTTYPE.GABE_OBJECTTYPE_FLOAT : self.ReadColFloat
                 }
    for idcol in range(0,len(self)):
        lstret.append([ self.GetColTitle(idcol)] + list(func_binding[coltypes[idcol]](idcol)))
    return lstret                    

#setattr(Gabe_rw,"ToList",ToList)
