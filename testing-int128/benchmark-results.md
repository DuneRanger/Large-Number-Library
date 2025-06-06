# New results

## 02

UNIQUE TEST NUMBERS PER BENCHMARK: 5000 (INCLUDING 0, 1, -1) \
TOTAL CALCULATIONS PER TEST: 25000000 \
TIMES AVERAGED OVER 10 DIFFERENT BENCHMARK ITERATIONS \
INITIAL RANDSTATE: 1 \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
|                | ADDITION       | SUBTRACTION    | MULTIPLICATION | DIVISION       | MODULO         | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| Boost int128   | 0.08074584     | 0.11106420     | 0.08760332     | 0.33198693     | 0.42102553     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| int128         | 0.03685005     | 0.06372738     | 0.30396378     | 3.00969442     | 1.83576831     | \
+----------------+----------------+----------------+----------------+----------------+----------------+



# Old results

# Because I keep on forgetting

https://en.wikipedia.org/wiki/Template:Arithmetic_operations

# O0

UNIQUE TEST NUMBERS PER BENCHMARK: 5000 (INCLUDING 0, 1, -1) \
TOTAL CALCULATIONS PER TEST: 25000000 \
TIMES AVERAGED OVER 10 DIFFERENT BENCHMARK ITERATIONS \
INITIAL RANDSTATE: 1 \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
|                | ADDITION       | SUBTRACTION    | MULTIPLICATION | DIVISION       | MODULO         | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| Boost int128   | 0.98110871     | 1.02015833     | 0.80478504     | 1.32210981     | 1.28249622     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| baseInt128     | 0.30994782     | 0.64063914     | 36.96767436    | 21.28791957    | 17.84794296    | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| testInt128     | 0.30190535     | 0.63405507     | 16.31455487    | 17.25728588    | 16.97780828    | \
+----------------+----------------+----------------+----------------+----------------+----------------+

# O1

Note: There were some negative values measured (iteration 8, testInt128), due to subtracting the lime for an empty loop
So it seems that an empty loop has a large enough variability, it can somehow take longer than the multiplication?
In any case, the final average values were positive, so I'll leave it be, but it is good to keep in mind the inaccuracy

UNIQUE TEST NUMBERS PER BENCHMARK: 5000 (INCLUDING 0, 1, -1) \
TOTAL CALCULATIONS PER TEST: 25000000 \
TIMES AVERAGED OVER 10 DIFFERENT BENCHMARK ITERATIONS \
INITIAL RANDSTATE: 1 \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
|                | ADDITION       | SUBTRACTION    | MULTIPLICATION | DIVISION       | MODULO         | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| Boost int128   | 0.06689085     | 0.10267233     | 0.04791149     | 0.31337562     | 0.34031611     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| baseInt128     | 0.02336389     | 0.05080474     | 13.58964967    | 6.32827500     | 5.40607307     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| testInt128     | 0.01348011     | 0.04044010     | 8.19736979     | 5.14625666     | 5.10587486     | \
+----------------+----------------+----------------+----------------+----------------+----------------+


# O2

UNIQUE TEST NUMBERS PER BENCHMARK: 5000 (INCLUDING 0, 1, -1) \
TOTAL CALCULATIONS PER TEST: 25000000 \
TIMES AVERAGED OVER 10 DIFFERENT BENCHMARK ITERATIONS \
INITIAL RANDSTATE: 1 \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
|                | ADDITION       | SUBTRACTION    | MULTIPLICATION | DIVISION       | MODULO         | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| Boost int128   | 0.07244347     | 0.11525488     | 0.08998094     | 0.33086974     | 0.41447884     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| baseInt128     | 0.03311574     | 0.06518117     | 11.93659594    | 6.48505058     | 5.38260191     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| testInt128     | 0.04411051     | 0.06250922     | 7.25177187     | 5.20329132     | 4.97309852     | \
+----------------+----------------+----------------+----------------+----------------+----------------+

# O3

UNIQUE TEST NUMBERS PER BENCHMARK: 5000 (INCLUDING 0, 1, -1) \
TOTAL CALCULATIONS PER TEST: 25000000 \
TIMES AVERAGED OVER 10 DIFFERENT BENCHMARK ITERATIONS \
INITIAL RANDSTATE: 1 \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
|                | ADDITION       | SUBTRACTION    | MULTIPLICATION | DIVISION       | MODULO         | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| Boost int128   | 0.07332592     | 0.11366490     | 0.05812595     | 0.37475996     | 0.37636016     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| baseInt128     | 0.03166889     | 0.06335435     | 11.67113973    | 6.29663805     | 5.66792172     | \
+----------------+----------------+----------------+----------------+----------------+----------------+ \
| testInt128     | 0.03346164     | 0.06228233     | 6.48288339     | 5.00088336     | 5.14258265     | \
+----------------+----------------+----------------+----------------+----------------+----------------+