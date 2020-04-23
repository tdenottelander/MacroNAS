# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

import os
import sys
import time
import shutil

import torchvision
import torch.nn
import torch
import numpy as np
import time
from torchsummary import summary
from thop.profile import profile

import utils

def run_evaluation(model, ensemble_model, data_loaders, args, save_model = '', load_model = ''):
    
    all_values = {} 
    device = 'cuda'
        
    utils.setup_torch(args['seed'])

    inputs = torch.randn((1,args['input_channels'],args['img_size'], args['img_size']))
    total_ops, total_params = profile(model, (inputs,), verbose=True)
    all_values['MMACs'] = np.round(total_ops / (1000.0**2), 2)
    all_values['Params'] = int(total_params)
    print (all_values)

    start = time.time()
    model = model.to(device)
    ensemble_model = ensemble_model.to(device)
    print ('models to device', time.time() - start)
    
    if len(load_model) > 0:
        model.load_state_dict(torch.load(os.path.join(args['dir'], load_model)))

    criterion = torch.nn.CrossEntropyLoss()    

    ################################################
    
    summary(model, (3, 32, 32), batch_size=args['batch_size'], device='cuda')

    criterion = torch.nn.CrossEntropyLoss().to(device)
    optimizer = torch.optim.SGD(
        model.parameters(),
        lr=args['lr_init'],
        momentum=0.9,
        weight_decay=1e-4
    )

    lrs = []
    n_models = 0

    all_values['epoch'] = []
    all_values['overall_time'] = []
    all_values['lr'] = []

    all_values['tr_loss'] = []
    all_values['tr_acc'] = []
    
    all_values['val_loss_single'] = []
    all_values['val_acc_single'] = []
    all_values['val_loss_ensemble'] = []
    all_values['val_acc_ensemble'] = []
    
    all_values['test_loss_single'] = []
    all_values['test_acc_single'] = []
    all_values['test_loss_ensemble'] = []
    all_values['test_acc_ensemble'] = []
    
    n_models = 0
    time_start = time.time()

    for epoch in range(args['epochs']):
        time_ep = time.time()
        
        lr = utils.get_cyclic_lr(epoch, lrs, args['lr_init'], args['lr_start_cycle'], args['cycle_period'])
        #print ('lr=%.3f' % lr)
        utils.set_learning_rate(optimizer, lr)
        lrs.append(lr)
        
        train_res = utils.train_epoch(device, data_loaders['train'], model, criterion, optimizer, args['num_samples_train'])
        
        values = [epoch + 1, lr, train_res['loss'], train_res['accuracy']]        
           
        if (epoch+1) >= args['lr_start_cycle'] and (epoch + 1) % args['cycle_period'] == 0:

            all_values['epoch'].append(epoch+1)
            all_values['lr'].append(lr)
            
            all_values['tr_loss'].append(train_res['loss'])
            all_values['tr_acc'].append(train_res['accuracy'])

            val_res = utils.evaluate(device, data_loaders['val'], model, criterion, args['num_samples_val'])        
            test_res = utils.evaluate(device, data_loaders['test'], model, criterion, args['num_samples_test'])

            all_values['val_loss_single'].append(val_res['loss'])
            all_values['val_acc_single'].append(val_res['accuracy'])
            all_values['test_loss_single'].append(test_res['loss'])
            all_values['test_acc_single'].append(test_res['accuracy'])
            
            utils.moving_average_ensemble(ensemble_model, model, 1.0 / (n_models + 1))
            utils.bn_update(device, data_loaders['train_for_bn_recalc'], ensemble_model)
            n_models += 1

            val_res = utils.evaluate(device, data_loaders['val'], ensemble_model, criterion, args['num_samples_val'])        
            test_res = utils.evaluate(device, data_loaders['test'], ensemble_model, criterion, args['num_samples_test'])

            all_values['val_loss_ensemble'].append(val_res['loss'])
            all_values['val_acc_ensemble'].append(val_res['accuracy'])
            all_values['test_loss_ensemble'].append(test_res['loss'])
            all_values['test_acc_ensemble'].append(test_res['accuracy'])

            overall_training_time = time.time()-time_start
            all_values['overall_time'].append(overall_training_time)
            
        #print (epoch, 'epoch_time', time.time() - time_ep)

        overall_training_time = time.time()-time_start
        #print ('overall time', overall_training_time)
        
        #print (all_values)

    if len(save_model) > 0:
        torch.save(ensemble_model.state_dict(), os.path.join(args['dir'], save_model + '_ensemble'))
        torch.save(model.state_dict(), os.path.join(args['dir'], save_model))

    return all_values

def run_evaluation_single_model(model, data_loaders, args, load_model = ''):
    
    all_values = {} 
    device = 'cuda'
    
    #this makes training deterministic
    utils.setup_torch(args['seed'])

    inputs = torch.randn((1,args['input_channels'],args['img_size'], args['img_size']))
    total_ops, total_params = profile(model, (inputs,), verbose=True)
    all_values['MMACs'] = np.round(total_ops / (1000.0**2), 2)
    all_values['Params'] = int(total_params)
    print (all_values)

    start = time.time()
    model = model.to(device)
    #ensemble_model = ensemble_model.to(device)
    print ('models to device', time.time() - start)
    
    if len(load_model) > 0:
        model.load_state_dict(torch.load(os.path.join(args['dir'], load_model)))

    #summary(model, (3, 32, 32), batch_size=args['batch_size'], device='cuda')

    criterion = torch.nn.CrossEntropyLoss()    

    ################################################
    
    #summary(model, (3, 32, 32), batch_size=args['batch_size'], device='cuda')
    criterion = torch.nn.CrossEntropyLoss().to(device)
    optimizer = torch.optim.SGD(
        model.parameters(),
        lr=args['lr_init'],
        momentum=0.9,
        weight_decay=1e-4
    )

    lrs = []

    all_values['epoch'] = []
    all_values['overall_time'] = []
    all_values['lr'] = []

    all_values['tr_loss'] = []
    all_values['tr_acc'] = []
    
    all_values['val_loss'] = []
    all_values['val_acc'] = []    
    all_values['test_loss'] = []
    all_values['test_acc'] = []
    
    n_models = 0
    time_start = time.time()

    for epoch in range(args['epochs']):
        time_ep = time.time()
        
        lr = utils.get_cyclic_lr(epoch, lrs, args['lr_init'], args['lr_start_cycle'], args['cycle_period'])
        #print (epoch, 'lr=%.5f' % lr)
        utils.set_learning_rate(optimizer, lr)
        lrs.append(lr)
        
        train_res = utils.train_epoch(device, data_loaders['train'], model, criterion, optimizer, args['num_samples_train'])
        
        values = [epoch + 1, lr, train_res['loss'], train_res['accuracy']]        
           
        if epoch == args['epochs'] - 1:

            all_values['epoch'].append(epoch+1)
            all_values['lr'].append(lr)
            
            all_values['tr_loss'].append(train_res['loss'])
            all_values['tr_acc'].append(train_res['accuracy'])

            val_res = utils.evaluate(device, data_loaders['val'], model, criterion, args['num_samples_val'])        
            test_res = utils.evaluate(device, data_loaders['test'], model, criterion, args['num_samples_test'])

            all_values['val_loss'].append(val_res['loss'])
            all_values['val_acc'].append(val_res['accuracy'])
            all_values['test_loss'].append(test_res['loss'])
            all_values['test_acc'].append(test_res['accuracy'])
            
            overall_training_time = time.time()-time_start
            all_values['overall_time'].append(overall_training_time)
            
        #print (epoch, 'epoch_time', time.time() - time_ep)

        overall_training_time = time.time()-time_start
        print ('overall time', overall_training_time)
        
        print (all_values)

    return all_values
