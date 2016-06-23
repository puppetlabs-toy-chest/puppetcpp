notice case $operatingsystem {
  'Solaris':          { include role::solaris } # apply the solaris class
  'RedHat', 'CentOS': { include role::redhat  } # apply the redhat class
  /^(Debian|Ubuntu)$/:{ include role::debian  } # apply the debian class
  default:            { include role::generic } # apply the generic class
}

notice case true {
    default: {}
}
