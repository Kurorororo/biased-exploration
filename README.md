# Biased Exploration for Satisficing Heuristic Search

This repository is for our ICAPS 2022 paper, [Biased Exploration for Satisficing Heuristic Search](https://tidel.mie.utoronto.ca/pubs/biased-exploration-icaps22.pdf).

## Classical Planning
Our implementation is on top of [Fast Downward](https://www.fast-downward.org/)

### Build
```bash
cd downward
python3 build.py
```

### Run

- Type-LAMA with Softmin-Type(h) using two type-based buckets
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hlm=lmcount(lm_factory=lm_rhw(reasonable_orders=true),transform=adapt_costs(one),pref=false)' --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'lazy(alt([single(hff),single(hff,pref_only=true),softmin_type_based([hff,g]),single(hlm),single(hlm,pref_only=true),softmin_type_based([hlm,g])],boost=1000),preferred=[hff,hlm],cost_type=one,reopen_closed=false)'
```

- Type-LAMA with Softmin-Type(h)
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hlm=lmcount(lm_factory=lm_rhw(reasonable_orders=true),transform=adapt_costs(one),pref=false)' --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'lazy(alt([single(hff),single(hff,pref_only=true),single(hlm),single(hlm,pref_only=true),softmin_type_based([hff,g])],boost=1000),preferred=[hff,hlm],cost_type=one,reopen_closed=false)'
```

- Softmin-Type(h) with FF
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'eager(alt([single(hff), softmin_type_based([hff, g()], ignore_size=true)]), cost_type=one)'
```

- Lin-Type(h) with FF
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'eager(alt([single(hff), linear_weighted_type_based([hff, g()])]), cost_type=one)
```

- 3-Type(h) with FF
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'eager(alt([single(hff), nth_type_based([hff, g()], n=3)]), cost_type=one)'
```

- Type(h) with FF
```bash
python3 fast-downward.py domain.pddl problem.pddl --evaluator 'hff=ff(transform=adapt_costs(one))' --search 'eager(alt([single(hff), softmin_type_based([hff, g()], ignore_size=true, ignore_weights=true)]))'
```

## Synthetic Data


```bash
python3 random_digraph.py -o result.json
```
