Foo produces Sql {
}

Foo::Bar produces Foo::Sql {
    bar => $bar,
    baz => foo()
}

Foo consumes Sql {
}

Foo::Bar consumes Foo::Sql {
    bar => $bar,
    baz => foo()
}

application lamp {
}

application lamp() {
}

application lamp($x) {
}

application lamp($x, $y) {
}

application lamp(Integer $x) {
}

application lamp(Integer $x, String[0, 1] $y = 'X') {
    notice hi
    $two = 1 + 1
    $three = $two + 1
}

Mysql::Db produces Sql {
  user      => $user,
  password  => $password,
  host      => pick($::mysql_host_override, $::fqdn),
  #port     => not used here, will default as described in the definition
  database  => $dbname,
  type      => 'mysql',
}

application lamp (
  $db_user,
  $db_password,
  $docroot = '/var/www/html',
) {

  lamp::web { $name:
    docroot => $docroot,
    export  => Http["lamp-${name}"],
  }

  lamp::app { $name:
    docroot => $docroot,
    consume => Sql["lamp-${name}"],
  }

  lamp::db { $name:
    db_user     => $db_user,
    db_password => $db_password,
    export      => Sql["lamp-${name}"],
  }

}

site {
}

site {
  lamp { example:
    wp_db_user      => foo,
    wp_db_password  => bar,
    nodes           => {
      Node['database.vm'] => [Lamp::Db[example]],
      Node['appserver.vm'] => [Lamp::Web[example]]
    }
  }
}
