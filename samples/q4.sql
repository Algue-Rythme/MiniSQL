SELECT p.titre, e.nom
FROM "employes.csv" e, "projets.csv" p
WHERE e.dpt NOT IN (SELECT r.dpt
                    FROM "employes.csv" r
                    WHERE r.ide = p.responsable)
