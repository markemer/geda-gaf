v 20031019 1
N 19900 41000 18700 41000 4
{
T 18900 41100 5 10 1 1 0 0 1
netname=rockA
}
N 21700 41000 25400 41000 4
{
T 24200 41100 5 10 1 1 0 0 1
netname=rockB
}
C 18000 38800 1 0 0 in-1.sym
{
T 18000 39100 5 10 1 1 0 0 1
portname=A
}
C 25400 40900 1 0 0 out-1.sym
{
T 25400 41200 5 10 1 1 0 0 1
portname=B
}
N 18600 38900 18900 38900 4
{
T 18700 39200 5 10 1 1 0 0 1
netname=rockA
}
C 19900 40400 1 0 0 rock.sym
{
T 20300 41500 5 10 1 1 0 0 1
refdes=Urock
T 20400 41800 5 10 1 1 0 0 1
source=rock.sch
}