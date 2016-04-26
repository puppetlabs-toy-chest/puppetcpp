$a = [1, 2, 3]
[$b, $c, $d] = $a
[$e, [$f, $g, [$h]]] = [[foo], [$a, /bar/, [1.5]]]
$i = {n => foo, j => bar, k => baz }
[$j, $k] = $i
[$l, [$m, [$n]]] = [bar, [String, $i]]

notice $a
notice $b
notice $c
notice $d
notice $e
notice $f
notice $g
notice $h
notice $i
notice $j
notice $k
notice $l
notice $m
notice $n
