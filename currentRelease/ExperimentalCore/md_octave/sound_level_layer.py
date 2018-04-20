# -*- coding: cp1252 -*-
from __future__ import print_function #compatibilit� python 3.0
import os

class SoundLevelLayer(object):
    """
        Charge le niveaux sonore calcul�s par les autres m�thodes de calculs
    """
    def __init__(self):
        self.recsurf={}
        self.recp={}
    def LoadData(self,folderdirect,coreconf,ls):
        """
            folderdirect Dossier o� se trouve le r�sultat du calcul
            coreconf Classe de configuration du code de calcul
            ls Librairie libsimpa
        """
        othercorepath = coreconf.paths["workingdirectory"] + folderdirect + os.sep
        if os.path.exists(othercorepath):
            # Chargement des r�cepteurs ponctuels
            rpfile = othercorepath + "rp.gabe"
            reader = ls.Gabe_rw()
            if os.path.exists(rpfile) and reader.Load(rpfile.encode("utf-8")):
                data = reader.ToList()
                for rp in data:
                    self.recp[int(rp[0])] = rp[1:]
            # Chargement des r�cepteurs de surfaces
            for idrecsurf in coreconf.recepteurssurf.keys():
                # If surface receiver
                filerecsurf = othercorepath + ("rs%i.gabe" % (idrecsurf))
                if os.path.exists(filerecsurf) and reader.Load(filerecsurf.encode("utf-8")):
                    recsurf_values = []
                    for idcol in range(0, len(reader)):
                        recsurf_values.append(list(reader.ReadColFloat(idcol)))
                    self.recsurf[idrecsurf] = recsurf_values
                # If cut receiver
                filerecsurf = othercorepath + ("rscut%i.gabe" % (idrecsurf))
                if os.path.exists(filerecsurf) and reader.Load(filerecsurf.encode("utf-8")):
                    recsurf_values = []
                    for idcol in range(0, len(reader)):
                        recsurf_values.append(list(reader.ReadColFloat(idcol)))
                    self.recsurf[idrecsurf] = recsurf_values

