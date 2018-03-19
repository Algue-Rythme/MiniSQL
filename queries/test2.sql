(select gens.Plat from "database/gens.csv" gens where gens.Plat = gens.Plat)
union
(select plats.Plat from "database/plats.csv" plats where plats.Plat = plats.Plat);
