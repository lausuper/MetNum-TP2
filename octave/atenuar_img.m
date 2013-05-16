function out = atenuar_img(B, coefInicial, coefFinal, k)

D = imagen_a_dct(B);

% El par (x0, y0) indica el primer elemento de la diagonal actual
x0 = 1;
y0 = 1;

% El par (x, y) indica el coeficiente actual
x = x0;
y = y0;

for i = 1:length(B)^2
    % Atenúo coeficiente actual
    if(coefInicial <= i && i <= coefFinal)
       D(y, x) = D(y, x) * k;
    endif;

    % Avanzo diagonalmente al siguiente coeficiente
    x--;
    y++;

    % Verifico si llegué al final de la diagonal actual
    if(x < 1 || y > length(B))
        % Avanzo a la siguiente diagonal
        if(x0 < length(B))
            x0++;
        else
            y0++;
        endif

        % Avanzo al primer elemento de la nueva diagonal
        x = x0;
        y = y0;
    endif
end

out = dct_a_imagen(D);