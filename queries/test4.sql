(select t1.Nom, t2.Plat
    from "database/gens.csv" t1, "database/plats.csv" t2
    where t1.Plat = t2.Plat)
minus
(select t3.Nom, t4.Plat
    from "database/gens.csv" t3, "database/plats.csv" t4
    where t3.Plat = t4.Plat and t4.Sain <> "oui");
