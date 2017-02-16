# -*- coding: cp1252 -*-
from libsimpa import *
from vec3 import vec3


#Ajout de fonctionnalit� par rapport a la librairie en C++

###########################################################
## Gabe_rw

def ToList(self):
    """
        Retourne les donn�es sous forme de listes python
    """
    lstret=[]
    for idcol in range(0,self.size()):
        lstret.append([ self.GetColTitle(idcol)] + list(self.Index(idcol)))
    return lstret                    

setattr(Gabe_rw,"ToList",ToList)
