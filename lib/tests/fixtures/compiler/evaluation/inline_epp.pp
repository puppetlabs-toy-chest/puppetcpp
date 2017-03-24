notice inline_epp('hello <%= "world" -%>!')
notice inline_epp('hello <%= [foo, bar, world].filter |$x| { $x =~ /^w/ }[0] -%>!')
notice inline_epp('hello <%= $name -%>!', { name => world })
notice inline_epp(@(TEMPLATE), { name => world })
  <%- |String $name| -%>
  hello <%= $name -%>!
  |- TEMPLATE
