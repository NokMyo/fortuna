# CSV input format

Fortuna intentionally keeps the historical data provider separate from the ORACLE engine. This avoids coupling the program to one undocumented lottery web endpoint.

The native parser accepts either form below, one draw per line:

```csv
round,n1,n2,n3,n4,n5,n6,bonus
1,10,23,29,33,37,40,16
```

or:

```csv
round,date,n1,n2,n3,n4,n5,n6,bonus
1,2002-12-07,10,23,29,33,37,40,16
```

Headers are ignored automatically. Numbers may arrive unsorted; the parser sorts each valid row before aggregation. Rows with duplicate main numbers or values outside 1..45 are rejected.
