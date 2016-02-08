(define ply-mesh (read-ply "data/dragon_vrip.ply"))
(define brick-image (read-image "data/brick.jpg"))

(define-scene scene
  (block
    (transform! (t* (translate 15 400 0)
                    (rotate-x (* 1.1 pi))
                    (rotate-y (* 0.1 pi))))
    (material! 
      (let ((tex-map (xy-texture-map (t* (translate-y -200) (scale 1e-1))))
            (tex (image-texture 'image brick-image 
                                'map tex-map)))
        (matte-material 'refl tex 'sigma 0)))
    (add-mesh! ply-mesh))
  (block
    (transform! (t* (translate 200 -200 -300)
                    (rotate-x (* -0.8 pi))
                    (rotate-z (* -0.3 pi))))
    (material! (matte-material
                 'refl (checkerboard-texture
                         'map (xy-texture-map)
                         'edge 10
                         'even-check (s* white 0.3)
                         'odd-check (s* white 0.7))
                 'sigma 1))
    (add-shape! (sphere 100)))
  (add-light! (point-light (point -200 0 -800)
                           (s* 1e6 white))))

(render scene
        'x-res 800
        'y-res 800
        'samples-per-pixel 5
        'seed 42
        'output "output.png")
