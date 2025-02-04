/*
 * CHBAdDelegate.h
 * Chartboost
 *
 * Copyright 2019 Chartboost. All rights reserved.
 */

#import "CHBAd.h"

NS_ASSUME_NONNULL_BEGIN

// MARK: - Events

/*!
 @class CHBAdEvent
 @brief The base class from which all delegate events inherit from.
 @discussion Event objects are passed as parameters to all ad delegate methods to provide useful context.
 */
@interface CHBAdEvent : NSObject
/*!
 @brief The ad related to the event.
 */
@property (nonatomic, readonly) id<CHBAd> ad;
@end

/*!
 @class CHBCacheEvent
 @brief A CHBAdEvent subclass passed on cache-related delegate methods.
 */
@interface CHBCacheEvent : CHBAdEvent
@end

/*!
 @class CHBShowEvent
 @brief A CHBAdEvent subclass passed on show-related delegate methods.
 */
@interface CHBShowEvent : CHBAdEvent
@end

/*!
 @class CHBClickEvent
 @brief A CHBAdEvent subclass passed on click-related delegate methods.
 */
@interface CHBClickEvent : CHBAdEvent
/*!
 @brief The view controller used to present the viewer for the link associated with the click.
 @discussion This is the view controller you passed on the showFromViewController: call or a Chartboost ad view controller which was presented on top of it. If you called showFromViewController: passing a nil view controller this property will be nil too.
 You may use it to present your custom click confirmation gate if you implement the shouldConfirmClick:confirmationHandler: ad delegate method.
 */
@property (nonatomic, readonly, nullable) UIViewController *viewController;
@end


// MARK: - Errors

/*!
 @typedef NS_ENUM (NSUInteger, CHBCacheErrorCode)
 @brief Error codes for failed cache operations.
 */
typedef NS_ENUM(NSUInteger, CHBCacheErrorCode) {
    CHBCacheErrorCodeInternal = 0,
    CHBCacheErrorCodeInternetUnavailable = 1,
    CHBCacheErrorCodeNetworkFailure = 5,
    CHBCacheErrorCodeNoAdFound = 6,
    CHBCacheErrorCodeSessionNotStarted = 7,
    CHBCacheErrorCodeAssetDownloadFailure = 16
};

/*!
 @class CHBCacheError
 @brief An error object passed on cache-related delegate methods.
 */
@interface CHBCacheError : NSObject
/*! @brief Error code that indicates the failure reason. */
@property (nonatomic, readonly) CHBCacheErrorCode code;
@end


/*!
 @typedef NS_ENUM (NSUInteger, CHBShowErrorCode)
 @brief Error codes for failed show operations.
 */
typedef NS_ENUM(NSUInteger, CHBShowErrorCode) {
    CHBShowErrorCodeInternal = 0,
    CHBShowErrorCodeSessionNotStarted = 7,
    CHBShowErrorCodeAdAlreadyVisible = 8,
    CHBShowErrorCodeInternetUnavailable = 25,
    CHBShowErrorCodePresentationFailure = 33,
    CHBShowErrorCodeNoCachedAd = 34
};

/*!
 @class CHBShowError
 @brief An error object passed on show-related delegate methods.
 */
@interface CHBShowError : NSObject
/*! @brief Error code that indicates the failure reason. */
@property (nonatomic, readonly) CHBShowErrorCode code;
@end

/*!
 @typedef NS_ENUM (NSUInteger, CHBClickErrorCode)
 @brief Error codes for failed click operations.
 */
typedef NS_ENUM(NSUInteger, CHBClickErrorCode) {
    CHBClickErrorCodeUriInvalid = 0,
    CHBClickErrorCodeUriUnrecognized = 1,
    CHBClickErrorCodeConfirmationGateFailure = 2,
    CHBClickErrorCodeInternal = 3
};

/*!
 @class CHBClickError
 @brief An error object passed on click-related delegate methods.
 */
@interface CHBClickError : NSObject
/*! @brief Error code that indicates the failure reason. */
@property (nonatomic, readonly) CHBClickErrorCode code;
@end


// MARK: - Delegate

/*!
 @protocol CHBAdDelegate
 @brief The protocol which all Chartboost ad delegates inherit from.
 @discussion Provides methods to receive notifications related to an ad's actions and to control its behavior.
 */
@protocol CHBAdDelegate <NSObject>

@optional

/*!
 @brief Called after a cache call, either if an ad has been loaded from the Chartboost servers and cached, or tried to but failed.
 @param event A cache event with info related to the cached ad.
 @param error An error specifying the failure reason, or nil if the operation was successful.
 @discussion Implement to be notified of when an ad is ready to be shown after the cache method has been called.
 
 A typical implementation would look like this:
 @code
 - (void)didCacheAd:(CHBCacheEvent *)event error:(nullable CHBCacheError *)error {
    if (error) {
        // Handle error
    } else {
        [event.ad showFromViewController:self];
    }
 }
 @endcode
 */
- (void)didCacheAd:(CHBCacheEvent *)event error:(nullable CHBCacheError *)error;

/*!
 @brief Called after a showFromViewController: call, right before an ad is presented.
 @param event A show event with info related to the ad to be shown.
 @param error An error specifying the failure reason, or nil if the operation was successful.
 @discussion Implement to be notified of when an ad is about to be presented.
 
 A typical implementation would look like this:
 @code
 - (void)willShowAd:(CHBShowEvent *)event error:(nullable CHBShowError *)error {
    if (error) {
        // Handle error
    } else {
        // Maybe pause ongoing processes like video or gameplay.
    }
 }
 @endcode
 */
- (void)willShowAd:(CHBShowEvent *)event error:(nullable CHBShowError *)error;

/*!
 @brief Called after a showFromViewController: call, either if the ad has been presented and an ad impression logged, or if the operation failed.
 @param event A show event with info related to the ad shown.
 @param error An error specifying the failure reason, or nil if the operation was successful.
 @discussion Implement to be notified of when the ad presentation process has finished. Note that this method may be called more than once if some error occurs after the ad has been successfully shown.
 
 A common practice consists of caching an ad here so there's an ad ready for the next time you need to show it.
 Note that this is not necessary for banners with automaticallyRefreshesContent set to YES.
 @code
 - (void)didShowAd:(CHBShowEvent *)event error:(nullable CHBShowError *)error {
    if (error) {
        // Handle error, possibly resuming processes paused in willShowAd:error:
    } else {
        [event.ad cache];
    }
 }
 @endcode
 */
- (void)didShowAd:(CHBShowEvent *)event error:(nullable CHBShowError *)error;

/*!
 @brief Called whenever the user clicks an ad to give a chance to the developer to present a confirmation gate before the click is handled.
 @param event A click event with info related to the ad clicked.
 @param confirmationHandler A block to be executed only if the return value is YES. It takes a BOOL parameter that indicates if the confirmation gate was passed or not.
 @return YES if the handling of the triggering click should be paused for confirmation, NO if the click should be handled without confirmation.
 @warning If you return YES in your implementation make sure to execute the confirmationHandler at some point, since the ad flow will be paused until then.
 If you use the event's view controller to present a confirmation view make sure it has been dismissed by the time you execute the confirmation handler.
 @discussion If you return YES it is your responsibility to implement some confirmation method that triggers the execution of the confirmationHandler.
 
 If this method is not implemented clicks will be handled without confirmation.
 
 A typical implementation would look like this:
 @code
 - (BOOL)shouldConfirmClick:(CHBClickEvent *)event confirmationHandler:(void(^)(BOOL))confirmationHandler
    if (self.needsClickConfirmation) {
        MyAwesomeAgeGate *ageGate = [[MyAwesomeAgeGate alloc] initWithCompletion:^(BOOL confirmed) {
            [ageGate dismissViewControllerAnimated:YES completion:^{
                confirmationHandler(confirmed);
            }];
        }];
        [event.viewController presentViewController:ageGate animated:YES completion:nil];
        return YES;
    } else {
        return NO;
    }
 }
 @endcode
 */
- (BOOL)shouldConfirmClick:(CHBClickEvent *)event confirmationHandler:(void(^)(BOOL))confirmationHandler;

/*!
 @brief Called after an ad has been clicked.
 @param event A click event with info related to the ad clicked.
 @param error An error specifying the failure reason, or nil if the operation was successful.
 @discussion Implement to be notified of when an ad has been clicked.
 If the click does not result into the opening of a link an error will be provided explaning why.
 
 A typical implementation would look like this:
 @code
 - (void)didClickAd:(CHBClickEvent *)event error:(nullable CHBClickError *)error {
    if (error) {
        // Handle error
    } else {
        // Maybe pause ongoing processes like video or gameplay.
    }
 }
 @endcode
 */
- (void)didClickAd:(CHBClickEvent *)event error:(nullable CHBClickError *)error;

/*!
 @brief Called when the link viewer presented as result of an ad click has been dismissed.
 @param event A click event with info related to the ad clicked.
 @param error An error specifying the failure reason, or nil if the operation was successful.
 @discussion Implement to be notified of when an ad click has been handled.
 This can mean an in-app web browser or App Store app sheet has been dismissed, or that the user came back to the app after the link was opened on an external application.
 
 A typical implementation would look like this:
 @code
 - (void)didFinishHandlingClick:(CHBClickEvent *)event error:(nullable CHBClickError *)error {
    // Resume processes previously paused on didClickAd:error: implementation.
 }
 @endcode
 */
- (void)didFinishHandlingClick:(CHBClickEvent *)event error:(nullable CHBClickError *)error;

@end

NS_ASSUME_NONNULL_END
