select t1.Nom, t1.Plat
from "database/gens.csv" t1
where t1.Plat not in (select t2.Plat from "database/plats.csv" t2 where t2.Sain = "oui");
