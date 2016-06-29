inline_epp(@(TEMPLATE), { foo => 1})
  <%- | Integer $foo,
        Integer $bar
      | -%>
  |- TEMPLATE
