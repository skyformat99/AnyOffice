<?php
/*
 * Usage: php json2php.php 1.json 2.json 3.json
 */
function echo_dep($dep)
{
    foreach ($dep as $p) {
        if (intval($p) . '' == $p)
            echo "[$p]";
        else
            echo "[\"$p\"]";
    }
}

function foreach_array($arr, $dep)
{
    foreach ($arr as $k => $v) {
        $next_dep = $dep;
        array_push($next_dep, $k);
        if (is_array($v)) {
            foreach_array($v, $next_dep);
        } else {
            echo "<?php \$data";
            echo_dep($next_dep);
            echo " ?? \"\" ?>\n";
        }
    }
}

for ($p = 1; $p < $argc; $p++) {
    $obj = json_decode(file_get_contents($argv[$p]), true);
    foreach_array($obj, []);
}