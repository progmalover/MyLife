'use strict';

angular.module('manager', ['myApp.filters', 'myApp.services',
        'myApp.directives','myApp.controllers', 'ui.bootstrap']).
  config(['$routeProvider', function($routeProvider) {
      $routeProvider.
          when('/bridges/configure', {templateUrl: 'partials/config.html',
              controller: 'ConfigCtrl'}).
          when('/bridges/monitor', {templateUrl: 'partials/monitor.html',
              controller: 'MonitorCtrl'}).
          when('/help', {templateUrl: 'partials/help.html',
              controller: 'HelpCtrl'}).
          otherwise({redirectTo: '/bridges/configure'});
}]);
