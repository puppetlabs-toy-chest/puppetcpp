$composed = { "ñ" => "foo" }
notice $composed["ñ"]
notice $composed["n\u0303"]
notice $composed["N\u0303"]
$decomposed = { "n\u0303" => "foo" }
notice $composed["ñ"]
notice $composed["n\u0303"]
notice $composed["N\u0303"]
