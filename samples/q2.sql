SELECT e.dpt, e.nom
FROM "employes.csv" e
WHERE e.dpt IN ( SELECT s.dpt
                 FROM "employes.csv" s, "departements.csv" ds
                 WHERE ds.directeur = s.ide
                   AND e.dpt = ds.idd)
