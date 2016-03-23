File {
    content => @(CONTENT)
        hello
        world
        |- CONTENT
}

file { '/tmp/default':
}

file { '/tmp/different':
    content => different
}

class foo {
    File {
        ensure => file,
        owner => peter
    }

    file { '/tmp/foo':
    }
}

class bar inherits foo {
    File {
        ensure => directory
    }

    file { '/tmp/bar':
    }
}

include bar
