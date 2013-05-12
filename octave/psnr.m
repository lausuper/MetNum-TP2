function res = psnr(original, con_ruido)

ecm = sum((original - con_ruido).^2) / length(original);
res = 10 * log10(max(original)^2 / ecm);