$(document).ready(function(){
  $('#saveButton').click(function(){
    var requiredFields = ['#appID', '#appKey', '#baseUrl'];
    var redirect = "https://westonkd.github.io/panda-face/redirect";

    if(verify(requiredFields)) {
      localStorage.setItem('user_key', $('#appKey').val());
      localStorage.setItem('base_url', $('#baseUrl').val());
      localStorage.setItem('app_id', $('#appID').val());

      var loginUrl = $('#baseUrl').val() +
        "/login/oauth2/auth?client_id=" +
        $('#appID').val() +
        "&response_type=code" +
        "&redirect_uri=" + redirect;
        console.log(loginUrl);

      window.location = loginUrl;
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
