select t1.Nom, t2.Plat, t2.Sain
from "database/gens.csv" t1, "database/plats.csv" t2
where t2.Sain = "oui" and t1.Plat = t2.Plat;
