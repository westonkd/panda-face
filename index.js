$(document).ready(function(){
  $('#saveButton').click(function(){
    var requiredFields = ['#appID', '#appKey', '#baseUrl'];
    var redirect = "https://westonkd.github.io/panda-face/redirect";

    if(verify(requiredFields)) {
      // window.localStorage.setItem('user_key', $('#appKey').val());
      // window.localStorage.setItem('base_url', $('#baseUrl').val());
      // window.localStorage.setItem('app_id', $('#appID').val());

      var loginUrl = $('#baseUrl').val() +
        "/login/oauth2/auth?client_id=" +
        $('#appID').val() +
        "&response_type=code" +
        "&redirect_uri=" + redirect;

      var returnData = {
        return_type: 'app_info',
        client_id: $('#appID').val(),
        client_secret: $('#appKey').val(),
        redirect_uri: "https://westonkd.github.io/panda-face/redirect",
        login_url: loginUrl
      };

      window.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify(returnData));
    }
  });

  function verify(requiredFields) {
    $("input").css('border-color','');
    for (var i = 0; i < requiredFields.length; i++) {
      if ($(requiredFields[i]).val() === '') {
        var field = $(requiredFields[i]);
        $(field).css('border-color', 'red');
        $(field).focus();
        return false;
      }
    }
    return true;
  }
});
