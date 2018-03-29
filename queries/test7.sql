select t1.Nom, t2.Plat, t2.Prix
from "database/gens.csv" t1, "database/plats.csv" t2
where t1.Plat = t2.Plat
order by t2.Prix;
