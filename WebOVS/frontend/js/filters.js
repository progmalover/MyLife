'use strict';

angular.module('myApp.filters', []).
    filter('range', function() {
      return function(input, total) {
        total = parseInt(total);
        for (var i=0; i<total; i++) {
          input.push(i);
        }
        return input;
      };
    })
    .filter('fields', function() {
        return function(input) {
            var result = "";
            for (var field in input) {
                result += field + "=" + input[field] + ",";
            }
            return result;
        };
    })
    .filter('flagSet', function() {
        return function(input) {
            return (input) ? "send_flow_rem," : "";
        }
    });
